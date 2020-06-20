#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

#include "common.h"

int sem_get(int key) {
  int id = semget(key, 0, 0);
  if (id == -1) {
    printf("sem_get\n");
    print_error;
  }
  return id;
}

int sem_create(int key, int n_sems) {
  int id = semget(key, n_sems, 0600 | IPC_CREAT);
  if (id == -1) {
    printf("sem_create\n");
    print_error;
  }
  return id;
}

void sem_delete(int sem_id) {
  int r = semctl(sem_id, 0, IPC_RMID);
  if (r == -1) {
    printf("sem_delete\n");
    print_error;
  }
}

int sem_op(int sem_id, int sem_num, short op, int wait) {
  struct sembuf s;
  s.sem_num = sem_num;
  s.sem_op = op;
  s.sem_flg = wait ? 0 : IPC_NOWAIT;
  return semop(sem_id, &s, 1);
}

void sem_set(int sem_id, int sem_num, int val) {
  int r = semctl(sem_id, sem_num, SETVAL, val);
  if (r == -1) {
    printf("sem_set\n");
    print_error;
  }
}
