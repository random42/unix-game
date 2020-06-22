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

void on_exit() {
  wait_for_children();
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
  me = get_player(_game, id);
  shm_stop_read(mem);
}

void spawn_pawns() {
  shm_write(mem);
  int first_id = get_player_first_pawn(_game, id);
  for (int i = 0; i < _game->n_pawns; i++) {
    debug_count();
    player* p = get_player(_game, i);
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
  debug("I'm player %d\n", me->id);
  spawn_pawns();
}

void place_pawns() {

}

void play_round() {

}

square* create_strategy(pawn* pawn) {
  return NULL;
}

void wait_round_end() {

}

void term() {

}

int main(int argc, char* argv[]) {
  assert(argc > 0);
  id = atoi(argv[0]);
  init();
  atexit(on_exit);
  start();
}