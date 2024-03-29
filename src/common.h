#ifndef COMMON_H
#define COMMON_H

#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <errno.h>

#define max(x,y) (x > y ? x : y)
#define min(x,y) (x < y ? x : y)
#define print_error printf("%s\n",strerror(errno))
#define error(x, ...) printf("\n[%d] ERROR: ", getpid());printf(x, ##__VA_ARGS__);print_error;exit(EXIT_FAILURE);

#define TRUE 1
#define FALSE 0

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"


#endif
