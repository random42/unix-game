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

void find_me() {
  shm_read(mem);
  for (int i = 0; i < _game->n_players && me == NULL; i++) {
    player* p = _game->players[i];
    if (p->id == id) {
      me = p;
    }
  }
  if (me == NULL) {
    error("player %d not found\n", id);
  }
  shm_stop_read(mem);
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
  find_me();
}

void spawn_pawns() {
  shm_write(mem);
  for (int i = 0; i < _game->n_pawns; i++) {
    debug_count();
    pawn* p = me->pawns[i];
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