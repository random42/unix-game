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
#define debug(x, ...) if (DEBUG) {P;printf("[%d] ", getpid());printf(x, ##__VA_ARGS__);V;}

void debug_create(int key);
void debug_get(int key);
void debug_count();
void debug_close();
void debug_p();
void debug_v();

#endif
