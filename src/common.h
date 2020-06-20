#ifndef COMMON_H
#define COMMON_H

#include <errno.h>
#include <stdio.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

#define print_error printf("%s\n",strerror(errno))

#endif
