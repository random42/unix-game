#include "common.h"
#include "conf.h"
#include "process.h"
#include "shm.h"
#include "game.h"
#include "sem.h"
#include "msg.h"
#include "random.h"
#include "debug.h"
#include "player.h"

shm* mem;
game* _game;
player* me;
int game_sem;
int squares_sem;
int msg_queue;
int id;
int pid;

void sig_handler(int sig) {
  debug("SIGNAL: %d\n", sig);
}

void init() {
  pid = get_process_id();
  // inizializza la libreria random
  random_init();
  // inizializza le strutture ipc
  debug_get(SEM_DEBUG_KEY);
  game_sem = sem_get(SEM_GAME_KEY);
  squares_sem = sem_get(SEM_SQUARES_KEY);
  msg_queue = msg_init(MSG_KEY);
  mem = shm_get(SHM_KEY);
  _game = mem->ptr;
  shm_read(mem);
  // cerca il player a cui corrisponde
  me = get_player(_game, id);
  shm_stop_read(mem);
  // imposta l'handler per terminare
  set_signal_handler(SIGTERM, term, TRUE);
  set_signal_handler(SIGINT, term, TRUE);
  set_signal_handler(SIGABRT, term, TRUE);
  // se non imposto un handler non posso usare la wait_signal
  set_signal_handler(ROUND_END_SIGNAL, sig_handler, FALSE);
}

void spawn_pawns() {
  shm_write(mem);
  int pawn_id = get_player_first_pawn(_game, id)->id;
  for (int i = 0; i < _game->n_pawns; i++) {
    pawn* p = get_pawn(_game, pawn_id++);
    char id_string[5];
    sprintf(id_string, "%d", p->id);
    char* args[] = {id_string, NULL};
    int pid = fork_and_exec("./bin/pawn", args);
    p->pid = pid;
    set_process_group_id(pid, get_process_group_id());
  }
  shm_stop_write(mem);
}

void start() {
  debug("PLAYER_START\n");
  // debug("player_ppid: %d\n", get_process_group_id());
  spawn_pawns();
  placement_phase();
  while (TRUE) {
    play_round();
  }
}

square* choose_placement_square() {
  square* s = NULL;
  // prendo la prima casella che non ha una pedina
  while (s == NULL || s->pawn_id) {
    int index = random_int_range(0, get_n_squares(_game) - 1);
    s = get_square(_game, index);
  }
  return s;
}

void placement_phase() {
  int first_pawn_id = get_player_first_pawn(_game, me->id)->id;
  for (int round = 0; round < _game->n_pawns; round++) {
    int value = (round * _game->n_players) + me->id;
    sem_op(game_sem, SEM_PLACEMENT, -value, TRUE);
    debug("PLACEMENT_TURN: %d\n", value);
    shm_read(mem);
    // debug("Player %d is placing\n", me->id);
    pawn* p = get_pawn(_game, first_pawn_id + round);
    // debug("Pawn %d\n", p->id);
    square* s = choose_placement_square();
    shm_stop_read(mem);
    shm_write(mem);
    // piazza il pedina
    place_pawn(p, s);
    // acquisisce il lock della casella per indicare che è occupata
    sem_op(squares_sem, get_square_index(_game, s->x, s->y), -1, TRUE);
    shm_stop_write(mem);
    sem_op(game_sem, SEM_PLACEMENT, value + 1, TRUE);
  }
}

void send_strategies() {
  shm_read(mem);
  int pawn_id = get_player_first_pawn(_game, id)->id;
  for (int i = 0; i < _game->n_pawns; i++) {
    pawn* p = get_pawn(_game, pawn_id++);
    message msg;
    msg.mtype = p->pid;
    // scelgo una delle due strategie in modo casuale uniforme
    msg.strategy = random_int_range(0, 1);
    msg_send(msg_queue, &msg, TRUE);
  }
  shm_stop_read(mem);
}

void play_round() {
  // attende l'inizio del round
  sem_op(game_sem, SEM_ROUND_START, 0, TRUE);
  debug("PLAYER_START_ROUND\n");
  // sleep(1);
  // invia le strategie ai pedoni
  send_strategies();
  debug("PLAYER_READY\n");
  // decrementa il semaforo per segnalare che è pronto
  sem_op(game_sem, SEM_ROUND_READY, -1, TRUE);
  debug("PLAYER_WAIT\n");
  // attende il segnale di fine round o di terminazione
  infinite_sleep();
}

void term(int sig) {
  debug("PLAYER_EXIT\n");
  wait_for_children();
  exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
  assert(argc > 0);
  id = atoi(argv[0]);
  init();
  start();
}