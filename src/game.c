#include <sys/time.h>
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

long game_size(int n_players, int n_pawns, int board_height, int board_width) {
  int squares = board_height * board_width;
  int pawns = n_pawns * n_players;
  return sizeof(game) + 
    (squares * sizeof(square*)) + 
    (squares * sizeof(square)) +
    (n_players * sizeof(player*)) +
    (n_players * sizeof(player)) +
    (pawns * sizeof(pawn*)) +
    (pawns * sizeof(pawn));
}

int min_distance(square* s1, square* s2) {
  return abs(s1->x - s2->x) + abs(s1->y - s2->y);
}

square* get_square(game* game, int x, int y) {
  return game->squares[y * game->board_width + x];
}

void place_pawn(pawn* pawn, square* square) {
  pawn->square = square;
  square->pawn = pawn;
}

void move_pawn(pawn* pawn, square* square) {
  assert(pawn->moves_left > 0);
  pawn->square->pawn = NULL;
  pawn->square = square;
  square->pawn = pawn;
  pawn->moves_left--;
}

void remove_captured_flags(game* game) {
  square** squares = game->squares;
  int size = game->board_height * game->board_width;
  int i;
  for (i = 0; i < size; i++) {
    square* s = squares[i];
    if (s->has_flag && s->pawn != NULL) {
      s->has_flag = FALSE;
      s->pawn->player->points += s->flag_points;
      s->flag_points = 0;
    }
  }
}

void print_square(square* s) {
  // non ci puo' essere una pedina su una casella con bandiera
  // nei momenti in cui si printa lo stato
  if (s->has_flag) {
    printf("F%6d  ", s->flag_points);
  }
  else if (s->pawn != NULL) {
    printf("P%6d  ", s->pawn->player->id);
  }
  else {
    printf("   ---   ");
  }
}

void print_game_state(game* game) {
  int h = game->board_height;
  int w = game->board_width;
  printf("\nSTATO DEL GIOCO\n\n");
  printf("   ");
  for (int j = 0; j < w; j++) {
    printf(" %6d  ", j);
  }
  for (int i = 0; i < h; i++) {
    printf("\n\n");
    printf("%3d", i);
    for (int j = 0; j < w; j++) {
      square* s = get_square(game, j, i);
      print_square(s);
    }
  }
}

void print_stats(game* game) {
  printf("\n METRICHE DEL GIOCO\n\n");
  double game_time = elapsed_time(&game->start_time);
  int total_moves = game->max_pawn_moves * game->n_pawns;
  int total_points = 0;
  for (int i = 0; i < game->n_players; i++) {
    player* p = game->players[i];
    total_points += p->points;
    int moves_left = 0;
    for (int j = 0; j < game->n_pawns; j++) {
      pawn* pawn = p->pawns[j];
      moves_left += pawn->moves_left;
    }
    int moves_made = total_moves - moves_left;
    printf("Giocatore %d:\n\tMosse giocate: %d\n\tMosse rimaste: %d\n\tMosse utilizzate su mosse totali: %.2lf\n\tPunti ottenuti: %d\n\tPunti su mosse utilizzate: %.2lf",
      p->id, moves_made, moves_left, (double)moves_made / total_moves, p->points, (double)p->points / moves_made);
  }
  printf("Round giocati: %d", game->rounds);
  printf("Punti totali ottenuti dai giocatori: %d", total_points);
  printf("Tempo di gioco: %.3lf", game_time);
  printf("Punti totali su tempo di gioco: %.2lf", total_points / game_time);
}
