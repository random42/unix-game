#include "common.h"
#include "conf.h"
#include "process.h"
#include "shm.h"
#include "game.h"
#include "sem.h"
#include "msg.h"
#include "random.h"
#include "debug.h"
#include "pawn.h"

shm* mem;
game* _game;
pawn* me;
int game_sem;
int squares_sem;
int msg_queue;
int id;
int pid;
int round_ended;

void round_end(int sig) {
  debug("PAWN_ROUND_END_SIGNAL\n");
  round_ended = TRUE;
}

void init() {
  pid = get_process_id();
  random_init();
  debug_get(SEM_DEBUG_KEY);
  game_sem = sem_get(SEM_GAME_KEY);
  squares_sem = sem_get(SEM_SQUARES_KEY);
  msg_queue = msg_init(MSG_KEY);
  mem = shm_get(SHM_KEY);
  _game = mem->ptr;
  shm_read(mem);
  /* cerca la pedina a cui corrisponde */
  me = get_pawn(_game, id);
  shm_stop_read(mem);
  /* imposta l'handler per terminare */
  set_signal_handler(SIGTERM, term, TRUE);
  set_signal_handler(SIGINT, term, TRUE);
  set_signal_handler(SIGABRT, term, TRUE);
  /* il segnale di fine round può essere interrotto da altri segnali */
  set_signal_handler(ROUND_END_SIGNAL, round_end, FALSE);
}

void start() {
  /* debug("pawn_ppid: %d\n", get_process_group_id()); */
  debug("PAWN_START\n");
  while (TRUE) {
    play_round();
  }
}

void play_round() {
  round_ended = FALSE;
  debug("PAWN_ROUND_START\n");
  message msg;
  msg_receive(msg_queue, &msg, TRUE);
  debug("STRATEGY: %s\n", msg.strategy ? "extern" : "center");
  sem_op(game_sem, SEM_ROUND_READY, 0, TRUE);
  debug("PAWN_START_PLAY\n");
  /* gioco */
  play(msg.strategy);
  debug("PAWN_ROUND_END round_ended: %d\n", round_ended);
}

void play(int strategy) {
  /* gioco finché il round non finisce */
  while (!round_ended) {
    /* debug("PAWN_PLAY\n"); */
    /* debug("1\n"); */
    shm_read(mem);
    /* scelgo la casella target in base alla strategia */
    square* target = NULL;
    if (strategy == CENTER_STRATEGY) {
      target = most_centered_controlled_flag(_game, me);
    }
    else if (strategy == EXTERN_STRATEGY) {
      target = most_extern_controlled_flag(_game, me);
    }
    shm_stop_read(mem);
    /* se target è null significa che non controllo bandiere */
    if (target != NULL) {
      debug("MOVE_TOWARDS (%d,%d) FROM (%d, %d)\n", target->x, target->y, me->x, me->y);
      move_towards(target);
    }
    else {
      /* se non controllo bandiere attendo min_hold_nsec  */
      /* in modo che qualche pedina si muova */
      /* nel caso al prossimo ciclo controllo una bandiera */
      /* debug("PAWN_SLEEPING\n"); */
      nano_sleep(_game->min_hold_nsec);
    }
  }
  debug("PAWN_PLAY_END\n");
}

/* sceglie la casella in cui muovere per andare verso la casella target */
/* se possibile sceglie una casella senza altre pedine */
square* choose_next_square(square* target) {
  shm_read(mem);
  /* ci sono massimo due possibili direzioni da scegliere */
  square* choices[2];
  int i = 0;
  square* from = get_pawn_square(_game, me);
  int x = from->x;
  int y = from->y;
  /* prendo le due caselle di scelta tra le 4 mosse disponibili */
  if (x > target->x) {
    choices[i++] = get_square_in(_game, x-1, y);
  }
  if (x < target->x) {
    choices[i++] = get_square_in(_game, x+1, y);
  }
  if (y > target->y) {
    choices[i++] = get_square_in(_game, x, y-1);
  }
  if (y < target->y) {
    choices[i++] = get_square_in(_game, x, y+1);
  }
  if (!(i >= 1 && i <= 2)) {
    debug("i = %d, from (%d,%d) to (%d,%d)\n", i, x, y, target->x, target->y);
  }
  /* verifico di avere massimo una o due mosse */
  assert(i >= 1 && i <= 2);
  square* choice;
  /* caso in cui ci sia solo una potenziale mossa */
  if (i == 1) {
    choice = choices[0];
  }
  /* caso con due mosse */
  /* scelgo la casella che non ha un pedone oppure ne scelgo una a caso */
  else {
    if (has_pawn(choices[0])) {
      choice = choices[1];
    }
    else if (has_pawn(choices[1])) {
      choice = choices[0];
    }
    else {
      int j = random_int_range(0, 1);
      choice = choices[j];
    }
  }
  shm_stop_read(mem);
  return choice;
}

/* fa una mossa verso la casella target */
void move_towards(square* target) {
  shm_read(mem);
  /* scelgo la casella in cui muovere */
  square* next = choose_next_square(target);
  shm_stop_read(mem);
  /* muovo */
  move_to(next);
}

void move_to(square* s) {
  /* debug("MOVING TO: (%d,%d)\n", s->x, s->y); */
  int r;
  square* from = get_pawn_square(_game, me);
  /* prendo il lock della casella di arrivo */
  debug("WAITING_SQUARE\n");
  r = sem_op(squares_sem, get_square_index(_game, s->x, s->y), -1, TRUE);
  if (r == -1) {
    /* se va in errore significa che  */
    /* è arrivato il segnale di fine round */
    /* e quindi non faccio la mossa */
    debug("SQUARE_LOCK semop: %s\n", strerror(errno));
    return;
  }
  /* rilascio il lock della casella di partenza */
  sem_op(squares_sem, get_square_index(_game, from->x, from->y), 1, TRUE);
  shm_write(mem);
  /* mi salvo se la casella ha una bandiera */
  int captured_flag = has_flag(s);
  /* muovo la pedina */
  move_pawn(_game, me, s);
  /* qua devo prendere esplicitamente il lock di debug */
  /* perche' print_game_state usa printf e non debug */
  debug("\n");
  debug_p();
  print_game_state(_game);
  debug_v();
  shm_stop_write(mem);
  /* se ho catturato una bandiera mando il segnale al master */
  /* conosco il suo pid perché è il mio process group id */
  /* (mentre il parent pid è quello del player) */
  if (captured_flag) {
    debug("FLAG_CAPTURED\n");
    int master_pid = get_process_group_id();
    message msg;
    msg.mtype = master_pid;
    msg_send(msg_queue, &msg, TRUE);
  }
  /* debug("MOVED TO: (%d,%d)\n", s->x, s->y); */
  /* effettuo la nanosleep prima di continuare a muovermi */
  nano_sleep(_game->min_hold_nsec);
}

void term(int sig) {
  debug("PAWN_EXIT\n");
  exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
  assert(argc > 0);
  id = atoi(argv[0]);
  init();
  start();
}
