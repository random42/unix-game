#include <signal.h>
#include "debug.h"
#include "sig.h"

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