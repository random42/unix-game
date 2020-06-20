#include "sem.h"
#include "debug.h"

#define PRINT 0

static int sem;

void debug_create(int key) {
  sem = sem_create(key, 1);
  sem_set(sem, PRINT, 1);
}

void debug_get(int key) {
  sem = sem_get(key);
}

void debug_close() {
  sem_delete(sem);
}

void debug_p() {
  sem_op(sem, PRINT, -1, 1);
}

void debug_v() {
  sem_op(sem, PRINT, 1, 1);
}
