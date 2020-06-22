
#include <sys/ipc.h>
#include <sys/shm.h>
#include "common.h"
#include "shm.h"
#include "sem.h"
#include "debug.h"

static int LOCK_SEMS = 4;
static int WRITE_LOCK = 0; // lock di scrittura
static int READ_LOCK = 1; // lock per modificare il numero di readers
static int READERS = 2; // numero di lettori
static int WRITE_WAIT = 3; // scrittori in attesa

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
  int sem_id = sem_create(key + 1, LOCK_SEMS);
  // imposta i semafori ai valori iniziali
  sem_set(sem_id, READ_LOCK, 1);
  sem_set(sem_id, WRITE_LOCK, 1);
  sem_set(sem_id, READERS, 0);
  sem_set(sem_id, WRITE_WAIT, 0);
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
  int sem_id = sem_get(key + 1);
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
  sem_op(shm->sem_id, WRITE_WAIT, 0, TRUE);
  sem_op(shm->sem_id, READ_LOCK, -1, TRUE);
  sem_op(shm->sem_id, READERS, 1, TRUE);
  int readers = sem_get_value(shm->sem_id, READERS);
  if (readers == 1) {
    sem_op(shm->sem_id, WRITE_LOCK, -1, TRUE);
  }
  sem_op(shm->sem_id, READ_LOCK, 1, TRUE);
}

void shm_stop_read(shm* shm) {
  sem_op(shm->sem_id, READ_LOCK, -1, TRUE);
  sem_op(shm->sem_id, READERS, -1, TRUE);
  int readers = sem_get_value(shm->sem_id, READERS);
  if (readers == 0) {
    sem_op(shm->sem_id, WRITE_LOCK, 1, TRUE);
  }
  sem_op(shm->sem_id, READ_LOCK, 1, TRUE);
}

void shm_write(shm* shm) {
  sem_op(shm->sem_id, WRITE_WAIT, 1, TRUE);
  sem_op(shm->sem_id, WRITE_LOCK, -1, TRUE);
  sem_op(shm->sem_id, WRITE_WAIT, -1, TRUE);
}

void shm_stop_write(shm* shm) {
  sem_op(shm->sem_id, WRITE_LOCK, 1, TRUE);
}

void shm_delete(shm* shm) {
  if (shmctl(shm->id, IPC_RMID, NULL) == -1) {
    debug("shmctl\n");
  }
  free(shm);
}

