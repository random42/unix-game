#ifndef GAME_H
#define GAME_H

struct pawn;

typedef struct flag {
  int points;
  square* square;
} flag;

typedef struct player {
  int n_pawns;
  struct pawn** pawns;
} player;

typedef struct square {
  struct pawn* pawn;
  int x;
  int y;
} square;

typedef struct pawn {
  int moves_left;
  player* player;
  square* square;
} pawn;

typedef struct board {
  int height;
  int width;
  square** squares;
} board;

typedef struct game {
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
  player** players;
  board* board;
} game;

#endif
