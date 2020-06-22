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

// non posso utilizzare "game"
// perche' identifica il tipo
game* _game;
shm* mem;
int game_sem;
int squares_sem;
int msg_queue;

void on_exit() {
  wait_for_children();
  msg_close(msg_queue);
  shm_delete(mem);
  sem_delete(game_sem);
  sem_delete(squares_sem);
  debug_close();
}

void init_game() {
  char* s = getenv("SO_NUM_G");
  // per essere sicuri di aver impostato l'ambiente del processo
  if (s == NULL) {
    printf("Ricordati di impostare le variabili d'ambiente!\n");
    exit(EXIT_FAILURE);
  }
  int n_players = atoi(getenv("SO_NUM_G"));
  int n_pawns = atoi(getenv("SO_NUM_P"));
  int max_time = atoi(getenv("SO_MAX_TIME"));
  int board_height = atoi(getenv("SO_ALTEZZA"));
  int board_width = atoi(getenv("SO_BASE"));
  int flag_min = atoi(getenv("SO_FLAG_MIN"));
  int flag_max = atoi(getenv("SO_FLAG_MAX"));
  int round_score = atoi(getenv("SO_ROUND_SCORE"));
  int max_pawn_moves = atoi(getenv("SO_N_MOVES"));
  int min_hold_nsec = atoi(getenv("SO_MIN_HOLD_NSEC"));
  mem = shm_create(SHM_KEY, get_game_size(n_players, n_pawns, board_height, board_width));
  _game = create_game(mem->ptr, n_players, n_pawns, max_time, board_height, board_width, flag_min, flag_max, round_score, max_pawn_moves, min_hold_nsec);
}

void spawn_players() {
  shm_write(mem);
  for (int i = 1; i <= _game->n_players; i++) {
    player* p = get_player(_game, i);
    // passo l'id del giocatore come argomento del processo
    // e quindi devo convertirlo in stringa
    char id_string[5];
    sprintf(id_string, "%d", p->id);
    char* args[] = {id_string, NULL};
    int pid = fork_and_exec("./bin/player", args);
    p->pid = pid;
    set_process_group_id(pid, get_process_group_id());
  }
  shm_stop_write(mem);
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
  spawn_players();
}

void play_round() {

}

void place_flags() {

}

void wait_players() {

}

void wait_flag_captures() {

}

void end_round() {

}

void end_game() {
  
}

int main(int argc, char* argv[]) {
  init();
  atexit(on_exit);
  start();
}