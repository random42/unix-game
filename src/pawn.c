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

void sig_handler(int sig) {
  debug("SIGNAL: %d\n", sig);
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
  // ignora il segnale di bandiera catturata
  set_signal_handler(FLAG_CAPTURED_SIGNAL, SIG_IGN, TRUE);
  // imposta l'handler per terminare
  // al segnale di fine del gioco
  set_signal_handler(GAME_END_SIGNAL, term, TRUE);
  set_signal_handler(ROUND_END_SIGNAL, sig_handler, TRUE);
}

void start() {
  // debug("pawn_ppid: %d\n", get_process_group_id());
  debug("PAWN_START\n");
  play_round();
  // attende la fine del gioco
  wait_signal(GAME_END_SIGNAL);
  term(GAME_END_SIGNAL);
}

void play_round() {
  message msg;
  msg_receive(msg_queue, &msg, TRUE);
  debug("MSG: %d\n", msg.move);
  sem_op(game_sem, SEM_ROUND_READY, 0, TRUE);
  if (msg.move) {
    play();
  } else {
    wait_signal(ROUND_END_SIGNAL);
  }
}

void play() {

}

Direction choose_direction(square* target) {

}

void move(square* square) {

}

void wait_round_end() {

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