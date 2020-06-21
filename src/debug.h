#ifndef DEBUG_H
#define DEBUG_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include "common.h"

#define DEBUG 1

#define P debug_p()
#define V debug_v()
#define debug(x, ...) if (DEBUG) {P;printf("%d ", getpid());printf(x, ##__VA_ARGS__);V;}
#define error(x, ...) P;printf("\n%d ERROR\n", getpid());printf(x, ##__VA_ARGS__);print_error;V;exit(EXIT_FAILURE);

void debug_create(int key);
void debug_get(int key);
void debug_close();
void debug_p();
void debug_v();

#endif
