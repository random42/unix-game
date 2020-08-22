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
  // cerca la pedina a cui corrisponde
  me = get_pawn(_game, id);
  shm_stop_read(mem);
  // imposta l'handler per terminare
  // al segnale di fine del gioco
  set_signal_handler(GAME_END_SIGNAL, term, TRUE);
  set_signal_handler(ROUND_END_SIGNAL, round_end, TRUE);
}

void start() {
  // debug("pawn_ppid: %d\n", get_process_group_id());
  debug("PAWN_START\n");
  while (TRUE) {
    play_round();
  }
}

void play_round() {
  debug("PAWN_ROUND_START\n");
  message msg;
  msg_receive(msg_queue, &msg, TRUE);
  debug("MSG: %d\n", msg.move);
  sem_op(game_sem, SEM_ROUND_READY, 0, TRUE);
  if (msg.move) {
    // gioco
    play();
    // aspetto il segnale di fine round 
    // se ho smesso di muovermi prima del segnale
    // altrimenti potrei ricevere il segnale mentre
    // aspetto il messaggio del prossimo round
    // e la ricezione fallirebbe
    if (!round_ended) {
      wait_round_end();
    }
  } else {
    wait_round_end();
  }
}

void play() {
  int play = TRUE;
  round_ended = FALSE;
  while (play && !round_ended) {
    shm_read(mem);
    // scelgo come obiettivo la bandiera piu' lontana da lcentro
    square* target = furthest_controlled_flag_from_center(_game, me);
    shm_stop_read(mem);
    if (target != NULL) {
      // nel caso in cui un'altra pedina abbia già mosso
      // potrebbe succedere che al momento della scelta del target
      // questa pedina non controlli 
      move_towards(target);
    }
    shm_read(mem);
    // continuo a muovermi se controllo altre bandiere
    play = pawn_controls_any_flag(_game, me);
    shm_stop_read(mem);
  }
  debug("PAWN_PLAY_END round_ended: %d\n", round_ended);
}

square* choose_next_square(square* target) {
  shm_read(mem);
  // ci sono massimo due possibili direzioni da scegliere
  square* choices[4];
  int i = 0;
  square* from = get_pawn_square(_game, me);
  int x = from->x;
  int y = from->y;
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
  if (!(i > 0 && i <= 2)) {
    debug("i = %d, from (%d,%d) to (%d,%d)\n", i, x, y, target->x, target->y);
  }
  assert(i > 0 && i <= 2);
  square* choice;
  // caso in cui ci sia solo una potenziale mossa
  if (i == 1) {
    choice = choices[0];
  }
  // caso con due mosse
  // scelgo la casella che non ha un pedone oppure ne scelgo una a caso
  else {
    if (has_pawn(choices[0])) {
      choice = choices[1];
    }
    else if (has_pawn(choices[1])) {
      choice = choices[0];
    }
    else {
      double r = random_zero_to_one();
      if (r >= 0.5) {
        choice = choices[0];
      }
      else choice = choices[1];
    }
  }
  shm_stop_read(mem);
  return choice;
}

void move_towards(square* target) {
  // la pedina si muove finché non si trova nella casella target
  shm_read(mem);
  int move = target->pawn_id != me->id;
  shm_stop_read(mem);
  while (move && !round_ended) {
    // scelgo la casella in cui muovere
    square* next = choose_next_square(target);
    // muovo
    move_to(next);
    shm_read(mem);
    // smetto di muovermi se sono arrivato alla casella target
    move = target->pawn_id != me->id;
    shm_stop_read(mem);
  }
}

void move_to(square* s) {
  debug("MOVING TO: (%d,%d)\n", s->x, s->y);
  int r;
  square* from = get_pawn_square(_game, me);
  // prendo il lock della casella di arrivo
  r = sem_op(squares_sem, get_square_index(_game, s->x, s->y), -1, TRUE);
  if (r == -1) {
    debug("SQUARE_LOCK semop: %s\n", strerror(errno));
    return;
  }
  // rilascio il lock della casella di partenza
  sem_op(squares_sem, get_square_index(_game, from->x, from->y), 1, TRUE);
  shm_write(mem);
  // mi salvo se la casella ha una bandiera
  int captured_flag = has_flag(s);
  // muovo la pedina
  move_pawn(_game, me, s);
  //debug
  debug_p();
  print_game_state(_game);
  debug_v();
  shm_stop_write(mem);
  // se ho catturato una bandiera mando il segnale al master
  // conosco il suo pid perché è il mio process group id
  // (mentre il parent pid è quello del player)
  if (captured_flag) {
    debug("FLAG_CAPTURED\n");
    int master_pid = get_process_group_id();
    message msg;
    msg.mtype = master_pid;
    msg_send(msg_queue, &msg, TRUE);
  }
  debug("MOVED TO: (%d,%d)\n", s->x, s->y);
  // effettuo la nanosleep prima di continuare a muovermi
  nano_sleep(_game->min_hold_nsec);
}

void wait_round_end() {
  wait_signal(ROUND_END_SIGNAL);
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