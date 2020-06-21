#include <signal.h>

#include "common.h"
#include "conf.h"
#include "random.h"
#include "shm.h"
#include "debug.h"
#include "timer.h"
#include "sig.h"
#include "sem.h"
#include "master.h"

int n_players;
int n_pawns;
int max_time;
int board_height;
int board_width;
int flag_min;
int flag_max;
int round_score;
int max_pawn_moves;
int min_hold_nsec;

game* g;

shm* mem;

// game* create_game(
//   void* ptr,
//   int n_players,
//   int n_pawns,
//   int max_time,
//   int board_height,
//   int board_width,
//   int flag_min,
//   int flag_max,
//   int round_score,
//   int max_pawn_moves,
//   int min_hold_nsec
//   );


void init() {

  mem = shm_create();
}

int main(int argc, char* argv[]) {
  init();
  start();
}