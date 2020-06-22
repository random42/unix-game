#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include "random.h"
#include "sem.h"
#include "game.h"
#include "shm.h"
#include "conf.h"
#include "debug.h"
#include "timer.h"
#include "process.h"

void timeout_handler(int sig) {
  printf("timeout %d\n", sig);
}

void test_timeout() {
  set_timeout(timeout_handler, 4, 100, 1);
  sleep(2);
  printf("slept 2\n");
  clear_timeout();
  sleep(3);
  printf("slept 3\n");
}
game* create_game(void* ptr, int n_players, int n_pawns, int max_time, int board_height, int board_width, int flag_min, int flag_max, int round_score, int max_pawn_moves, int min_hold_nsec);

void test_game() {
  int n_players = 3;
  int n_pawns = 3;
  int max_time = 3;
  int board_height = 4;
  int board_width = 6;
  int flag_min = 5;
  int flag_max = 10;
  int round_score = 20;
  int max_pawn_moves = 5;
  int min_hold_nsec = 100;
  shm* mem = shm_create(SHM_KEY, get_game_size(n_players, n_pawns, board_height, board_width));
  game* g = create_game(mem->ptr, n_players, n_pawns, max_time, board_height, board_width, flag_min, flag_max, round_score, max_pawn_moves, min_hold_nsec);
  int flag_points = 25;
  pawn* p1 = get_player_first_pawn(g, 1);
  pawn* p2 = get_player_first_pawn(g, 2);
  pawn* p3 = get_player_first_pawn(g, 3);
  square* s1 = get_square_in(g, 0, 2);
  square* s2 = get_square_in(g, 3, 1);
  square* s3 = get_square_in(g, 5, 3);
  square* s4 = get_square_in(g, 0, 0);
  place_pawn(p1, s1);
  place_pawn(p2, s2);
  place_pawn(p3, s3);
  place_flag(s4, flag_points);
  print_game_state(g);
  assert(squares_distance(s1, s3) == 6);
  assert(square_controls(g, s1, get_square_in(g, 1, 1)));
  assert(square_controls(g, s2, get_square_in(g, 1, 1)));
  assert(square_controls(g, s1, get_square_in(g, 2, 2)));
  assert(square_controls(g, s2, get_square_in(g, 2, 2)));
  assert(square_controls(g, s2, get_square_in(g, 2, 1)));
  assert(!square_controls(g, s1, get_square_in(g, 2, 1)));
  assert(pawn_controls(g, p1, get_square_in(g, 1, 1)));
  assert(pawn_controls(g, p1, get_pawn_square(g, p1)));
  p1->moves_left = 0;
  assert(pawn_controls(g, p1, get_pawn_square(g, p1)));
  assert(!pawn_controls(g, p1, get_square_in(g, 1, 1)));
  p1->moves_left = 2;
  print_game_state(g);
  move_pawn(g, p1, get_square_in(g, 0, 1));
  print_game_state(g);
  move_pawn(g, p1, get_square_in(g, 0, 0));
  remove_captured_flags(g);
  print_game_state(g);
  player* pl1 = get_player(g, p1->player_id);
  assert(pl1->id == p1->player_id);
  assert(pl1->points == flag_points);
  print_game_stats(g);
}

int main() {
  debug_create(SEM_DEBUG_KEY);
  printf("sizeof game %d\n", sizeof(game));
  printf("sizeof player %d\n", sizeof(player));
  printf("sizeof square %d\n", sizeof(square));
  printf("sizeof pawn %d\n", sizeof(pawn));
  test_game();
  // void* x = 0;
  // void* y = x + sizeof(game);
  // printf("x %d, y %d, diff %d\n", x, y, y-x);
}
