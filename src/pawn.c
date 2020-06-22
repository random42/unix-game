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

void on_exit() {
}

void find_me() {
  shm_read(mem);
  int player_pid = get_parent_process_id();
  player* my_player = NULL;
  for (int i = 0; i < _game->n_players && my_player == NULL; i++) {
    player* pl = _game->players[i];
    if (pl->pid == player_pid) {
      my_player = pl;
    }
  }
  if (my_player == NULL) {
    error("Player with pid %d not found\n", player_pid);
  }
  for (int j = 0; j < _game->n_pawns && me == NULL; j++) {
    pawn* p = my_player->pawns[j];
    if (p->id == id) {
      me = p;
    }
  }
  if (me == NULL) {
    error("Pawn with id %d not found\n", id);
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

void start() {
  debug("I'm pawn %d\n", me->id);
}

void play_round() {

}

square* get_target() {

}

void play() {

}

Direction choose_direction(square* target) {

}

void move(square* square) {

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