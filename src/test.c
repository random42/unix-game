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

game* create_game(int n_players, int n_pawns, int max_time, int board_height, int board_width, int flag_min, int flag_max, int round_score, int max_pawn_moves, int min_hold_nsec) {
  shm* mem = shm_create(SHM_KEY, get_game_size(n_players, n_pawns, board_height, board_width));
  game* g = shm_alloc(mem, sizeof(game));
  g->n_players = n_players;
  g->n_pawns = n_pawns;
  g->max_time = max_time;
  g->board_height = board_height;
  g->board_width = board_width;
  g->flag_min = flag_min;
  g->flag_max = flag_max;
  g->round_score = round_score;
  g->max_pawn_moves = max_pawn_moves;
  g->min_hold_nsec = min_hold_nsec;
  g->rounds_played = 0;
  int squares = board_height * board_width;
  g->squares = shm_alloc(mem, squares * sizeof(square*));
  int i;
  for (i = 0; i < squares; i++) {
    g->squares[i] = shm_alloc(mem, sizeof(square));
    square* s = g->squares[i];
    s->has_flag = FALSE;
    s->flag_points = 0;
    s->pawn = NULL;
    // la coordinata x è il resto della divisione
    s->x = i % board_width;
    // la coordinata y è il risultato della divisione
    s->y = i / board_width;
  }
  g->players = shm_alloc(mem, n_players * sizeof(player*));
  for (i = 0; i < n_players; i++) {
    g->players[i] = shm_alloc(mem, sizeof(player));
    player* p = g->players[i];
    p->pid = -1;
    p->id = i;
    p->points = 0;
    p->pawns = shm_alloc(mem, n_pawns * sizeof(pawn*));
    for (int j = 0; j < n_pawns; j++) {
      p->pawns[j] = shm_alloc(mem, sizeof(pawn));
      pawn* pn = p->pawns[j];
      pn->pid = -1;
      pn->moves_left = max_pawn_moves;
      pn->player = p;
      pn->square = NULL;
    }
  }
  return g;
}

void test_game() {
  game* g = create_game(3, 3, 3, 4, 6, 2, 3, 10, 10, 100);
  int flag_points = 321;
  pawn* p1 = g->players[0]->pawns[0];
  pawn* p2 = g->players[1]->pawns[0];
  pawn* p3 = g->players[2]->pawns[0];
  square* s1 = get_square(g, 0, 2);
  square* s2 = get_square(g, 3, 1);
  square* s3 = get_square(g, 5, 3);
  square* s4 = get_square(g, 0, 0);
  place_pawn(p1, s1);
  place_pawn(p2, s2);
  place_pawn(p3, s3);
  place_flag(s4, flag_points);
  assert(squares_distance(s1, s3) == 6);
  assert(square_controls(g, s1, get_square(g, 1, 1)));
  assert(square_controls(g, s2, get_square(g, 1, 1)));
  assert(square_controls(g, s1, get_square(g, 2, 2)));
  assert(square_controls(g, s2, get_square(g, 2, 2)));
  assert(square_controls(g, s2, get_square(g, 2, 1)));
  assert(!square_controls(g, s1, get_square(g, 2, 1)));
  assert(pawn_controls(g, p1, get_square(g, 1, 1)));
  assert(pawn_controls(g, p1, p1->square));
  p1->moves_left = 0;
  assert(pawn_controls(g, p1, p1->square));
  assert(!pawn_controls(g, p1, get_square(g, 1, 1)));
  p1->moves_left = 2;
  print_game_state(g);
  move_pawn(p1, get_square(g, 0, 1));
  print_game_state(g);
  move_pawn(p1, get_square(g, 0, 0));
  remove_captured_flags(g);
  assert(p1->player->points == flag_points);
  print_game_state(g);
  print_game_stats(g);
}

int main() {
  debug_create(SEM_DEBUG_KEY);
  test_game();
}
