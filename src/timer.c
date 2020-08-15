
#include <signal.h>
#include <sys/time.h>
#include "debug.h"
#include "timer.h"
#include "process.h"

void set_timeout(void (*handler)(int), int sec, int micros, int atomic) {
  set_signal_handler(SIGALRM, handler, atomic);
  struct itimerval timer;
  // setta il prossimo scattare del timer
  timer.it_value.tv_sec = sec;
  timer.it_value.tv_usec = micros;
  // questi sono per dire se bisogna ripetere il timer
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = 0;
  // starta il timer
  // REAL per dire di aspettare il tempo reale
  if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
    error("setitimer\n");
  }
}

void clear_timeout() {
  struct itimerval timer;
  // setta il prossimo scattare del timer
  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = 0;
  // questi sono per dire se bisogna ripetere il timer
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = 0;
  // starta il timer
  // REAL per dire di aspettare il tempo reale
  if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
    error("setitimer\n");
  }
}

