#ifndef COMMON_H
#define COMMON_H

#define TRUE 1
#define FALSE 0

#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>

#define print_error printf("%s\n",strerror(errno))

#endif
