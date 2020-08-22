#ifndef GAME_H
#define GAME_H

typedef enum Direction {UP = 0, DOWN = 1, RIGHT = 2, LEFT = 3} Direction; 

typedef struct square {
  // questo id vale 0 se non c'è una pedina, altrimenti è l'id della pedina
  int pawn_id;
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
int has_pawn(square* s);
// ritorna TRUE se la casella ha una bandiera e non ha un pedone
int has_flag(square* s);
// byte necessari per allocare il gioco
int get_game_size(int n_players, int n_pawns, int board_height, int board_width);
// inizializza il gioco nella memoria del puntatore
game* create_game(void* ptr, int n_players, int n_pawns, int max_time, int board_height, int board_width, int flag_min, int flag_max, int round_score, int max_pawn_moves, int min_hold_nsec);
// distanza minima (in mosse) tra due caselle
int squares_distance(square* s1, square* s2);
// ritorna la distanza tra una casella e il centro della scacchiera
// si usa un numero decimale per i casi in cui l'altezza o la larghezza
// siano numeri pari e non esiste quindi una casella centrale
double distance_from_center(game* g, square* s);
// ritorna TRUE se il pedone controlla la casella
int pawn_controls_square(game* g, pawn* p, square* target);
// ritorna TRUE se il pedone controlla almeno una bandiera
int pawn_controls_any_flag(game* g, pawn* p);
// scrive sul puntatore le caselle con bandiera controllate dal pedone
// e ne ritorna il numero
int get_controlled_flags(game* g, pawn* p, square** ptr);
// ritorna la casella con bandiera controllata dal pedone
// che è più distante dal centro della scacchiera
// NULL se il pedone non controlla bandiere
square* furthest_controlled_flag_from_center(game* g, pawn* p);
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
