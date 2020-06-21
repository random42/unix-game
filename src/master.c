#include <signal.h>
#include <sys/time.h>

#include "common.h"
#include "conf.h"
#include "random.h"
#include "shm.h"
#include "debug.h"
#include "timer.h"
#include "process.h"
#include "msg.h"
#include "sem.h"
#include "master.h"

game* _game;
shm* mem;
int game_sem;
int squares_sem;
int msg_queue;

void on_exit() {
  msg_close(msg_queue);
  shm_delete(mem);
  sem_delete(game_sem);
  sem_delete(squares_sem);
  debug_close();
}

game* create_game(int n_players, int n_pawns, int max_time, int board_height, int board_width, int flag_min, int flag_max, int round_score, int max_pawn_moves, int min_hold_nsec) {
  mem = shm_create(SHM_KEY, get_game_size(n_players, n_pawns, board_height, board_width));
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

void init_game() {
  char* s = getenv("SO_NUM_G");
  int n_players = atoi(getenv("SO_NUM_G"));
  int n_pawns = atoi(getenv("SO_NUM_P"));
  int max_time = atoi(getenv("SO_MAX_TIME"));
  int board_width = atoi(getenv("SO_BASE"));
  int board_height = atoi(getenv("SO_ALTEZZA"));
  int flag_min = atoi(getenv("SO_FLAG_MIN"));
  int flag_max = atoi(getenv("SO_FLAG_MAX"));
  int round_score = atoi(getenv("SO_ROUND_SCORE"));
  int max_pawn_moves = atoi(getenv("SO_N_MOVES"));
  int min_hold_nsec = atoi(getenv("SO_MIN_HOLD_NSEC"));
  _game = create_game(n_players, n_pawns, max_time, board_height, board_width, flag_min, flag_max, round_score, max_pawn_moves, min_hold_nsec);
}

void init() {
  random_init();
  debug_create(SEM_DEBUG_KEY);
  init_game();
  game_sem = sem_create(SEM_GAME_KEY, 3);
  squares_sem = sem_create(SEM_SQUARES_KEY, get_n_squares(_game));
  msg_queue = msg_init(MSG_KEY);
  // imposto l'id di questo processo come l'id del gruppo di processi
  // in modo da mandare facilmente segnali a tutti i sottoprocessi
  set_process_group_id(0, 0);
}

void start() {
  fork_and_exec("./bin/player", NULL);
}

int main(int argc, char* argv[]) {
  init();
  atexit(on_exit);
  start();
}