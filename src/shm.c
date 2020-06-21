
#include <sys/ipc.h>
#include <sys/shm.h>
#include "common.h"
#include "shm.h"
#include "sem.h"
#include "debug.h"

static int LOCK_SEMS = 3;
static int READ = 0; // num of readers
static int WRITE = 1; // 0 if writing, 1 otherwise
static int WRITE_WAIT = 2; // number of writers waiting

// create shared memory
shm* shm_create(int key, int size) {
  shm* shm = malloc(sizeof(shm));
  int id = shmget(key, size, 0600 | IPC_CREAT);
  if (id == -1) {
    error("shmget\n");
  }
  void* ptr = shmat(id, NULL, 0);
  if (ptr == (void*)-1) {
    error("shmat\n");
  }
  int sem_key = key;
  int sem_id = sem_create(sem_key, LOCK_SEMS);
  // writing lock is free at start
  sem_set(sem_id, WRITE, 1);
  shm->id = id;
  shm->size = size;
  shm->ptr = ptr;
  shm->free = ptr;
  shm->sem_id = sem_id;
  return shm;
}

shm* shm_get(int key) {
  shm* shm = malloc(sizeof(shm));
  int id = shmget(key, 0, 0600);
  if (id == -1) {
    error("shmget\n");
  }
  void* ptr = shmat(id, NULL, 0);
  if (ptr == (void*)-1) {
    error("shmat\n");
  }
  int sem_key = key;
  int sem_id = sem_get(sem_key);
  shm->id = id;
  shm->size = 0;
  shm->free = NULL;
  shm->ptr = ptr;
  shm->sem_id = sem_id;
  return shm;
}

int shm_get_free_space(shm* shm) {
  // verifico che il processo creatore stia allocando memoria
  assert(shm->free != NULL);
  return shm->size - (shm->free - shm->ptr);
}

void* shm_alloc(shm* shm, int bytes) {
  // debug("[shm] size %d, free %d, bytes %d\n", shm->size, shm_get_free_space(shm), bytes);
  assert(bytes > 0);
  // verifico che il processo creatore stia allocando memoria
  assert(shm->free != NULL);
  // verifico che ci sia abbastanza spazio da allocare
  assert(shm_get_free_space(shm) >= bytes);
  void* r = shm->free;
  shm->free += bytes;
  return r;
}

void shm_read(shm* shm) {
  // waiting for writers to write
  sem_op(shm->sem_id, WRITE_WAIT, 0, TRUE);
  // increase readers by 1
  sem_op(shm->sem_id, READ, 1, TRUE);
}

void shm_stop_read(shm* shm) {
  // decrease readers by 1
  sem_op(shm->sem_id, READ, -1, TRUE);
}

void shm_write(shm* shm) {
  // increase writers waiting by 1
  sem_op(shm->sem_id, WRITE_WAIT, 1, TRUE);
  // wait for readers to go to 0
  sem_op(shm->sem_id, READ, 0, TRUE);
  // get writing lock
  sem_op(shm->sem_id, WRITE, -1, TRUE);
}

void shm_stop_write(shm* shm) {
  // release writing lock
  sem_op(shm->sem_id, WRITE, 1, TRUE);
  // decrease writers waiting by 1
  sem_op(shm->sem_id, WRITE_WAIT, -1, TRUE);
}

void shm_delete(shm* shm) {
  if (shmctl(shm->id, IPC_RMID, NULL) == -1) {
    debug("shmctl\n");
  }
  free(shm);
}

