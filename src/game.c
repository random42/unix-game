#include <sys/time.h>
#include <math.h>
#include "common.h"
#include "game.h"

double elapsed_time(struct timeval* from) {
  double start = from->tv_sec+((double)from->tv_usec/1e6);
  double now;
  struct timeval x;
  gettimeofday(&x, NULL);
  unsigned int seconds = x.tv_sec;
  unsigned int microseconds = x.tv_usec;
  now = seconds+((double)microseconds/1e6);
  return now-start;
}

int get_n_squares(game* g) {
  return g->board_height * g->board_width;
}

pawn* get_first_pawn(game* g) {
  return (void*)get_first_player(g) + sizeof(player) * g->n_players;
}

player* get_first_player(game* g) {
  return (void*)get_first_square(g) + sizeof(square) * get_n_squares(g);
}

square* get_first_square(game* g) {
  return (void*)g + sizeof(game);
}

pawn* get_pawn(game* g, int id) {
  // gli id partono da 1 mentre gli indici da 0
  return &get_first_pawn(g)[id - 1];
}

player* get_player(game* g, int id) {
  return &get_first_player(g)[id - 1];
}

// x e y partono da 0
int get_square_index(game* g, int x, int y) {
  // l'indice della casella è il numero della riga 
  // per il numero di righe più il numero della colonna
  return (y * g->board_width) + x;
}

square* get_square(game* g, int i) {
  return &get_first_square(g)[i];
}

square* get_square_in(game* g, int x, int y) {
  return get_square(g, get_square_index(g, x, y));
}

pawn* get_player_first_pawn(game* g, int player_id) {
  pawn* p = get_first_pawn(g);
  while (p->player_id != player_id) {
    p = get_pawn(g, p->id + 1);
  }
  return p;
}

square* get_pawn_square(game* g, pawn* p) {
  return get_square(g, get_square_index(g, p->x, p->y));
}

int has_pawn(square* s) {
  return !!s->pawn_id;
}

int has_flag(square* s) {
  return s->has_flag && !has_pawn(s);
}

int get_game_size(int n_players, int n_pawns, int board_height, int board_width) {
  return sizeof(game) + 
    (sizeof(square) * board_height * board_width) +
    (sizeof(player) * n_players) +
    (sizeof(pawn) * n_players * n_pawns);
}

game* create_game(void* ptr, int n_players, int n_pawns, int max_time, int board_height, int board_width, int flag_min, int flag_max, int round_score, int max_pawn_moves, int min_hold_nsec) {
  game* g = ptr;
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
  for (int i = 0; i < squares; i++) {
    square* s = get_square(g, i);
    s->has_flag = FALSE;
    s->flag_points = 0;
    s->pawn_id = 0;
    // la coordinata x è il resto della divisione
    s->x = i % board_width;
    // la coordinata y è il risultato della divisione
    s->y = i / board_width;
  }
  int player_id = 1;
  int pawn_id = 1;
  for (player_id = 1; player_id <= n_players; player_id++) {
    player* p = get_player(g, player_id);
    p->id = player_id;
    p->pid = 0;
    p->points = 0;
    for (int j = 0; j < n_pawns; j++) {
      pawn* pawn = get_pawn(g, pawn_id);
      pawn->id = pawn_id++;
      pawn->pid = 0;
      pawn->moves_left = max_pawn_moves;
      pawn->player_id = player_id;
      pawn->x = -1;
      pawn->y = -1;
    }
  }
  return g;
}

int squares_distance(square* s1, square* s2) {
  return abs(s1->x - s2->x) + abs(s1->y - s2->y);
}

double distance_from_center(game* g, square* s) {
  double x = g->board_width - 1;
  double y = g->board_height - 1;
  return fabs(x/2 - s->x) + fabs(y/2 - s->y);
}

int pawn_controls_square(game* g, pawn* p, square* target) {
  square* from = get_pawn_square(g, p);
  int distance = squares_distance(from, target);
  // se il pedone non ha abbastanza mosse per arrivare alla casella
  // allora non puo' controllarla a prescindere
  int controls = p->moves_left >= distance;
  for (int i = 0; i < get_n_squares(g) && controls; i++) {
    square* s = get_square(g, i);
    if (s != from && has_pawn(s) && squares_distance(s, target) < distance) {
      controls = FALSE;
    }
  }
  return controls;
}

int pawn_controls_any_flag(game* g, pawn* p) {
  square* from = get_pawn_square(g, p);
  int controls = FALSE;
  for (int i = 0; i < get_n_squares(g) && !controls; i++) {
    square* s = get_square(g, i);
    if (has_flag(s) && pawn_controls_square(g, p, s)) {
      controls = TRUE;
    }
  }
  return controls;
}

// TODO remove if possibile
int get_controlled_flags(game* g, pawn* p, square** ptr) {
  square* from = get_pawn_square(g, p);
  int controls = FALSE;
  int count = 0;
  for (int i = 0; i < get_n_squares(g); i++) {
    square* s = get_square(g, i);
    if (has_flag(s) && pawn_controls_square(g, p, s)) {
      ptr[count++] = s;
    }
  }
  return count;
}

square* furthest_controlled_flag_from_center(game* g, pawn* p) {
  square* from = get_pawn_square(g, p);
  square* target = NULL;
  double max_distance = -1;
  for (int i = 0; i < get_n_squares(g); i++) {
    square* s = get_square(g, i);
    double distance = distance_from_center(g, s);
    if (has_flag(s) && pawn_controls_square(g, p, s) && distance > max_distance) {
      max_distance = distance;
      target = s;
    }
  }
  return target;
}

void place_flag(square* square, int points) {
  square->flag_points = points;
  square->has_flag = TRUE;
}

void place_pawn(pawn* pawn, square* square) {
  pawn->x = square->x;
  pawn->y = square->y;
  square->pawn_id = pawn->id;
}

void move_pawn(game* g, pawn* pawn, square* to) {
  assert(pawn->moves_left > 0);
  square* from = get_pawn_square(g, pawn);
  assert(squares_distance(from, to) == 1);
  from->pawn_id = 0;
  to->pawn_id = pawn->id;
  pawn->x = to->x;
  pawn->y = to->y;
  pawn->moves_left--;
  if (to->has_flag) {
    to->has_flag = FALSE;
    player* pl = get_player(g, pawn->player_id);
    pl->points += to->flag_points;
    to->flag_points = 0;
  }
}

void remove_captured_flags(game* g) {
  for (int i = 0; i < get_n_squares(g); i++) {
    square* s = get_square(g, i);
    if (s->has_flag && s->pawn_id) { // se non e' 0 'pawn_id' vuol dire che c'e' un pedone
      s->has_flag = FALSE;
      pawn* p = get_pawn(g, s->pawn_id);
      player* pl = get_player(g, p->player_id);
      pl->points += s->flag_points;
      s->flag_points = 0;
    }
  }
}

void print_square(game* g, square* s) {
  // non ci puo' essere una pedina su una casella con bandiera
  // nei momenti in cui si printa lo stato
  // quindi si printa o F {punti della bandiera}
  // o P {id del player}
  if (has_flag(s)) {
    printf(ANSI_COLOR_RED" %2d  "ANSI_COLOR_RESET, s->flag_points);
  }
  else if (has_pawn(s)) {
    player* p = get_player(g, get_pawn(g, s->pawn_id)->player_id);
    // stampo l'id del player che controlla la pedina
    printf(ANSI_COLOR_GREEN" %2d  "ANSI_COLOR_RESET, p->id);
  }
  else {
    printf(ANSI_COLOR_BLUE"  x  "ANSI_COLOR_RESET);
  }
}

void print_game_state(game* g) {
  // si usa l'intero in mezzo a %d per stampare alla stessa altezza
  int h = g->board_height;
  int w = g->board_width;
  printf("\nSTATO DEL GIOCO\n\n");
  printf("     ");
  for (int j = 0; j < w; j++) {
    printf("%3d  ", j);
  }
  printf("\n\n");
  for (int i = 0; i < h; i++) {
    printf("%3d  ", i);
    for (int j = 0; j < w; j++) {
      square* s = get_square(g, get_square_index(g, j, i));
      print_square(g, s);
    }
    printf("\n\n");
  }
}

void print_game_stats(game* g) {
  printf("\n METRICHE DEL GIOCO\n\n");
  double game_time = elapsed_time(&g->start_time);
  int total_moves = g->max_pawn_moves * g->n_pawns;
  int total_points = 0;
  for (int i = 0; i < g->n_players; i++) {
    player* p = get_player(g, i + 1);
    total_points += p->points;
    int moves_left = 0;
    int first_pawn_id = get_player_first_pawn(g, p->id)->id;
    for (int j = 0; j < g->n_pawns; j++) {
      pawn* pawn = get_pawn(g, first_pawn_id + j);
      moves_left += pawn->moves_left;
    }
    int moves_made = total_moves - moves_left;
    printf("Giocatore %d:\n\tMosse giocate: %d\n\tMosse rimaste: %d\n\tMosse utilizzate su mosse totali: %.2lf\n\tPunti ottenuti: %d\n\tPunti su mosse utilizzate: %.2lf\n",
      p->id, moves_made, moves_left, (double)moves_made / total_moves, p->points, (double)p->points / moves_made);
  }
  printf("Round giocati: %d\n", g->rounds_played);
  printf("Punti totali ottenuti dai giocatori: %d\n", total_points);
  printf("Tempo di gioco: %.3lf secondi\n", game_time);
  printf("Punti totali su tempo di gioco: %.2lf\n", total_points / game_time);
}
