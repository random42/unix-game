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
#include "game.h"
#include "master.h"

// non posso utilizzare "game"
// perche' identifica il tipo
game* _game;
shm* mem;
int game_sem;
int squares_sem;
int msg_queue;

void end_game(int sig) {
  end_round();
  printf("\nTimeout!\nPosizione finale:\n");
  shm_read(mem);
  print_game_state(_game);
  print_game_stats(_game);
  shm_stop_read(mem);
  term();
}

void init_game() {
  // getenv: prende la stringa di una variabile d'ambiente
  // atoi: converte una stringa in un intero
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
  }
  shm_stop_write(mem);
}

void init() {
  random_init();
  debug_create(SEM_DEBUG_KEY);
  init_game();
  game_sem = sem_create(SEM_GAME_KEY, SEM_GAME_N);
  // imposto i semafori ai valori iniziali
  sem_set(game_sem, SEM_PLACEMENT, 0);
  sem_set(game_sem, SEM_ROUND_READY, _game->n_players);
  sem_set(game_sem, SEM_ROUND_START, 1);
  int n_squares = get_n_squares(_game);
  squares_sem = sem_create(SEM_SQUARES_KEY, n_squares);
  for (int i = 0; i < n_squares; i++) {
    sem_set(squares_sem, i, 1);
  }
  msg_queue = msg_init(MSG_KEY);
  // imposto l'id di questo processo come id del gruppo di processi
  // in modo da mandare facilmente segnali a tutti i processi figli
  set_process_group_id(0, 0);
  // ignora il segnale di fine round
  // in quanto viene mandato da questo processo
  set_signal_handler(ROUND_END_SIGNAL, SIG_IGN, TRUE);
  set_signal_handler(SIGABRT, term, TRUE);
  set_signal_handler(SIGINT, term, TRUE);
  set_signal_handler(SIGTERM, term, TRUE);
}

void start() {
  debug("MASTER\n");
  spawn_players();
  placement_phase();
  // set game start time
  gettimeofday(&_game->start_time, NULL);
  while (TRUE) {
    play_round();
  }
  // debug("NORMAL_TERM\n");
  // end_game(0);
}

void placement_phase() {
  printf("Fase di posizionamento...\n");
  sem_op(game_sem, SEM_PLACEMENT, 1, TRUE);
  int end = (_game->n_pawns * _game->n_players) + 1;
  sem_op(game_sem, SEM_PLACEMENT, -end, TRUE);
  printf("Fase di posizionamento terminata\n");
}

void end_round() {
  debug("ROUND_END\n");
  // annullo il timeout di fine gioco
  clear_timeout();
  // reimposta i semafori ai valori iniziali
  sem_op(game_sem, SEM_ROUND_START, 1, TRUE);
  sem_set(game_sem, SEM_ROUND_READY, _game->n_players);
  // manda il segnale di fine round a tutto il gruppo di processi
  send_signal(0, ROUND_END_SIGNAL);
  debug("ROUND_END_SENT\n");
  // sleep(1);
}

void play_round() {
  shm_write(mem);
  int round = ++_game->rounds_played;
  shm_stop_write(mem);
  printf("Inizia il round numero %d\n", round);
  int flags = place_flags();
  shm_read(mem);
  print_game_state(_game);
  shm_stop_read(mem);
  // fa iniziare il round
  debug("ROUND_START\n");
  // fa mandare le strategie dai giocatori
  sem_op(game_sem, SEM_ROUND_START, -1, TRUE);
  // reimposta il semaforo
  debug("ROUND_STARTED\n");
  // attende che i giocatori inviino le strategie
  sem_op(game_sem, SEM_ROUND_READY, 0, TRUE);
  debug("PLAYING!\n");
  printf("I giocatori hanno inviato le strategie, i pedoni cominciano a muoversi..\n");
  // imposto il timeout di fine gioco
  set_timeout(end_game, _game->max_time, 0, TRUE);
  // sleep(3);
  // attende i messaggi di cattura delle bandiere
  wait_flag_captures(flags);
  // termina il round
  end_round();
}

void wait_flag_captures(int flags) {
  int captured_flags = 0;
  while (captured_flags < flags) {
    message msg;
    msg_receive(msg_queue, &msg, TRUE);
    captured_flags++;
    printf("Flag captured! %d/%d\n", captured_flags, flags);
  }
}

int place_flags() {
  int target_score = _game->round_score;
  int score = 0;
  int flags = 0;
  while (score < target_score) {
    // punteggio della bandiera casuale
    int flag_points = random_int_range(_game->flag_min, _game->flag_max);
    // se arrivo al limite del round score arrotondo il punteggio
    flag_points = min(target_score - score, flag_points);
    square* s = NULL;
    // scelgo una casella libera da pedine e bandiere
    while (s == NULL || has_pawn(s) || has_flag(s)) {
      int index = random_int_range(0, get_n_squares(_game) - 1);
      s = get_square(_game, index);
    }
    place_flag(s, flag_points);
    score += flag_points;
    flags++;
  }
  assert(score == target_score);
  return flags;
}

void term() {
  debug("MASTER_EXIT\n");
  // manda il segnale di terminare agli altri processi
  // 0 per mandare al gruppo di processi
  send_signal(0, SIGTERM);
  // aspetta che terminino
  wait_for_children();
  // elimina le risorse ipc
  msg_close(msg_queue);
  shm_delete(mem);
  sem_delete(game_sem);
  sem_delete(squares_sem);
  debug_close();
  exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
  init();
  start();
}