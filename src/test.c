#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include "random.h"
#include "sem.h"
#include "shm.h"
#include "debug.h"
#include "timer.h"
#include "sig.h"

void timeout_handler(int sig) {
  printf("timeout %d\n", sig);
}

void test_timeout() {
  set_timeout(timeout_handler, 4, 100, 1);
  sleep(2);
  printf("slept 2\n");
  clear_timeout();
  sleep(3);
  printf("slept 3\n");
}

int main() {
}
