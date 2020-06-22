#ifndef GAME_H
#define GAME_H

typedef enum Direction {UP = 0, DOWN = 1, RIGHT = 2, LEFT = 3} Direction; 

typedef struct square {
  int pawn_id; // se > 0 allora identifica l'id della pedina
  int has_flag;
  int flag_points;
  int x;
  int y;
} square;

typedef struct player {
  int id;
  int pid;
  int points;
} player;

typedef struct pawn {
  int id;
  int pid;
  int moves_left;
  int player_id;
  // le coordinate sono -1 prima che la pedina venga piazzata
  int x;
  int y;
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
  int rounds_played;
  struct timeval start_time;
} game;

/*
Poiche' il gioco va in memoria condivisa le strutture sono piatte senza
puntatori. Lo spazio della memoria è suddiviso nel seguente modo:
game
square (board_height * board_width)
player (n_players)
pawn (n_players * n_pawns)

Si utilizzeranno delle funzioni per ricavare i puntatori
corretti degli oggetti.
*/

// numero di caselle totali
int get_n_squares(game* g);
// funzioni per avere i puntatori agli oggetti
pawn* get_first_pawn(game* g);
player* get_first_player(game* g);
square* get_first_square(game* g);
pawn* get_pawn(game* g, int id);
player* get_player(game* g, int id);
int get_square_index(game* g, int x, int y);
square* get_square(game* g, int i);
square* get_square_in(game* g, int x, int y);
pawn* get_player_first_pawn(game* g, int player_id);
square* get_pawn_square(game* g, pawn* p);
// byte necessari per allocare il gioco
int get_game_size(int n_players, int n_pawns, int board_height, int board_width);
// inizializza il gioco nella memoria del puntatore
game* create_game(void* ptr, int n_players, int n_pawns, int max_time, int board_height, int board_width, int flag_min, int flag_max, int round_score, int max_pawn_moves, int min_hold_nsec);
// distanza minima (in mosse) tra due caselle
int squares_distance(square* s1, square* s2);
// ritorna 1 se la casella from controlla la casella target
// in base alle pedine presenti in campo
// 0 altrimenti
int square_controls(game* g, square* from, square* target);
// stessa cosa ma controlla anche che la pedina abbia
// le mosse necessarie per arrivare alla casella
int pawn_controls(game* g, pawn* pawn, square* target);
// posiziona una bandiera nella casella
void place_flag(square* square, int points);
// posiziona la pedina nella casella
void place_pawn(pawn* pawn, square* square);
// muove la pedina nella casella
void move_pawn(game* g, pawn* pawn, square* to);
// rimuove tutte le bandiere dalle caselle in cui è presente una pedina
void remove_captured_flags(game* g);
void print_square(game* g, square* s);
// stampa lo stato del gioco
void print_game_state(game* g);
// stampa le metriche del gioco
void print_game_stats(game* g);

#endif
