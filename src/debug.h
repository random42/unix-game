#ifndef DEBUG_H
#define DEBUG_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include "common.h"

#define DEBUG 0

#define debug(x, ...) if (DEBUG) {debug_p();printf("[%d] ", getpid());printf(x, ##__VA_ARGS__);debug_v();}

void debug_create(int key);
void debug_get(int key);
void debug_count();
void debug_close();
void debug_p();
void debug_v();

#endif
