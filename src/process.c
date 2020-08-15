#include <signal.h>
#include <unistd.h>
#include <time.h>
#include "common.h"
#include "debug.h"
#include "process.h"

int nano_sleep(long ns) {
  // un secondo sono 1 miliardo di nanosecondi
  long one_sec = 1e9;
  struct timespec t;
  t.tv_sec = ns / one_sec;
  t.tv_nsec = ns % one_sec;
  return nanosleep(&t, NULL);
}

int get_process_id() {
  return getpid();
}

int get_parent_process_id() {
  return getppid();
}

int get_process_group_id() {
  return getpgid(0);
}

void set_process_group_id(int pid, int pgid) {
  int r = setpgid(pid, pgid);
  if (r == -1) {
    error("setpgid\n");
  }
}

void wait_for_children() {
  int child;
  while ((child = wait(NULL)) != -1) continue;
}

int fork_and_exec(char* path, char** argv) {
  int child = fork();
  switch(child) {
    case -1: { // error
      error("fork\n");
      break;
    }
    case 0: { // child process
      int r = execv(path, argv);
      if (r == -1) {
        error("execv\n");
      }
      break;
    }
    default: { // parent process
      return child;
    }
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

void send_signal(int pid, int sig) {
  int r = kill(pid, sig);
  if (r == -1) {
    error("send_signal\n");
  }
}

void wait_signal(int sig) {
  sigset_t set;
  // azzera il set di segnali
  sigemptyset(&set);
  // aggiunge il segnale al set di segnali
  sigaddset(&set, sig);
  int r = sigwait(&set, NULL);
  if (r == -1) {
    error("wait_sig\n");
  }
}