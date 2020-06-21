#include <signal.h>
#include <unistd.h>
#include "common.h"
#include "debug.h"
#include "process.h"

int get_process_group_id() {
  return getpgid(0);
}

void set_process_group_id(int pid, int pgid) {
  int r = setpgid(pid, pgid);
  if (r == -1) {
    error("setpgid\n");
  }
}

// set function to handle signals
// if atomic is TRUE then the handler cannot be interrupted by other signals
void set_signal_handler(int signal, void (*f)(int), int atomic) {
  struct sigaction s;
  s.sa_handler = f;
  if (atomic)
    sigfillset(&s.sa_mask);
  else
    sigemptyset(&s.sa_mask);
  int r = sigaction(signal, &s, NULL);
  if (r == -1) {
    error("sigaction\n");
  }
}

void send_sig(int pid, int sig) {
  int r = kill(pid, sig);
  if (r == -1) {
    error("send_sig\n");
  }
}

void wait_sig(int sig) {
  sigset_t set;
  // azzera il set di segnali
  sigemptyset(&set);
  // aggiungi il segnale al set di segnali
  sigaddset(&set, sig);
  int r = sigwait(&set, NULL);
  if (r == -1) {
    error("wait_sig\n");
  }
}