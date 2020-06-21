#ifndef GAME_H
#define GAME_H

enum Direction {UP = 0, DOWN = 1, RIGHT = 2, LEFT = 3}; 

struct pawn;
struct flag;

typedef struct square {
  struct pawn* pawn;
  int has_flag;
  int flag_points;
  int x;
  int y;
} square;

typedef struct player {
  int pid;
  int id;
  int n_pawns;
  struct pawn** pawns;
  int points;
} player;

typedef struct pawn {
  int pid;
  int moves_left;
  player* player;
  square* square;
} pawn;

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
  int rounds;
  struct timeval start_time;
  player** players;
  square** squares;
} game;

// byte necessari per allocare il gioco
long game_size(int n_players, int n_pawns, int board_height, int board_width);
// ritorna la casella delle coordinate
square* get_square(game* game, int x, int y);
// distanza minima (in mosse) tra due caselle
int min_distance(square* s1, square* s2);
// posiziona la pedina nella casella
void place_pawn(pawn* pawn, square* square);
// muove la pedina nella casella
void move_pawn(pawn* pawn, square* square);
// rimuove tutte bandiere dalle caselle con pedine sopra
void remove_captured_flags(game* game);
// stampa lo stato del gioco
void print_game_state(game* game);
// stampa le metriche del gioco
void print_stats(game* game);

#endif
