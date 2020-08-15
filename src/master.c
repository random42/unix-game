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
  // TODO assert validity of config
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
  }
  shm_stop_write(mem);
}

void init() {
  random_init();
  debug_create(SEM_DEBUG_KEY);
  init_game();
  game_sem = sem_create(SEM_GAME_KEY, 3);
  // imposto i semafori ai valori iniziali
  sem_set(game_sem, SEM_PLACEMENT, 0);
  sem_set(game_sem, SEM_ROUND_READY, _game->n_players);
  sem_set(game_sem, SEM_ROUND_START, 1);
  squares_sem = sem_create(SEM_SQUARES_KEY, get_n_squares(_game));
  msg_queue = msg_init(MSG_KEY);
  // imposto l'id di questo processo come l'id del gruppo di processi
  // in modo da mandare facilmente segnali a tutti i sottoprocessi
  set_process_group_id(0, 0);
  // ignora il segnale di fine gioco
  // in quanto è mandato da questo processo
  set_signal_handler(GAME_END_SIGNAL, SIG_IGN, TRUE);
}

void start() {
  spawn_players();
  placement_phase();
  // while (TRUE) {
    play_round();
  // }
  end_game();
}

void placement_phase() {
  printf("Fase di posizionamento...\n");
  sem_op(game_sem, SEM_PLACEMENT, 1, TRUE);
  int end = (_game->n_pawns * _game->n_players) + 1;
  sem_op(game_sem, SEM_PLACEMENT, -end, TRUE);
  printf("Fase di posizionamento terminata\n");
}

void play_round() {
  int round = _game->rounds_played;
  printf("Inizia il round numero %d\n", round);
  place_flags();
  print_game_state(_game);
  sleep(2);
  // fa iniziare il round
  debug("ROUND_START\n");
  // fa mandare le strategie dai giocatori
  sem_op(game_sem, SEM_ROUND_START, -1, TRUE);
  // attende che i giocatori inviino le strategie
  sem_op(game_sem, SEM_ROUND_READY, 0, TRUE);
  debug("PLAYERS_READY\n");
  sleep(4);
  debug("ROUND_END\n");
  send_signal(0, ROUND_END_SIGNAL);
}

void place_flags() {
  int target_score = _game->round_score;
  int score = 0;
  while (score < target_score) {
    // punteggio della bandiera casuale
    int flag_points = random_int_range(_game->flag_min, _game->flag_max);
    // se arrivo al limite del round score arrotondo il punteggio
    flag_points = min(target_score - score, flag_points);
    square* s = NULL;
    // scelgo una casella libera da pedine e bandiere
    while (s == NULL || s->pawn_id || s->has_flag) {
      int index = random_int_range(0, get_n_squares(_game) - 1);
      s = get_square(_game, index);
    }
    place_flag(s, flag_points);
    score += flag_points;
  }
  assert(score == target_score);
}

void wait_players() {
}

void wait_flag_captures() {
  
}

void end_round() {
}

void end_game() {
  sleep(4);
  debug("GAME_END\n");
  send_signal(0, GAME_END_SIGNAL);
  on_exit();
  debug("EXITING\n");
  exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
  init();
  start();
}