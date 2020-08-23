
#include <sys/ipc.h>
#include <sys/shm.h>
#include "common.h"
#include "shm.h"
#include "sem.h"
#include "debug.h"

// https://en.wikipedia.org/wiki/Readers%E2%80%93writers_problem#Third_readers%E2%80%93writers_problem

static int LOCK_SEMS = 4;
static int WRITE_LOCK = 0; // lock di scrittura
static int READERS_LOCK = 1; // lock per modificare il numero di readers
static int READERS = 2; // semaforo che conta il numero di lettori
static int QUEUE = 3; // lock che preserva l'ordine delle richieste di scrittura/lettura
static int _sem_id;

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
  // utilizzo la chiave della memoria condivisa + 1 come chiave per i semafori
  int sem_id = sem_create(key + 1, LOCK_SEMS);
  // imposta i semafori ai valori iniziali
  sem_set(sem_id, READERS_LOCK, 1);
  sem_set(sem_id, WRITE_LOCK, 1);
  sem_set(sem_id, READERS, 0);
  sem_set(sem_id, QUEUE, 1);
  shm->id = id;
  shm->size = size;
  shm->ptr = ptr;
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
  shm->ptr = ptr;
  shm->sem_id = sem_id;
  return shm;
}

void shm_read(shm* shm) {
  // debug("SHM_QUEUE\n");
  // debug("PRE shm_read ptr: %p sem_id: %d\n", shm, shm->sem_id, READERS);
  sem_op(shm->sem_id, QUEUE, -1, TRUE);
  sem_op(shm->sem_id, READERS_LOCK, -1, TRUE);
  sem_op(shm->sem_id, READERS, 1, TRUE);
  int readers = sem_get_value(shm->sem_id, READERS);
  if (readers == 1) {
    sem_op(shm->sem_id, WRITE_LOCK, -1, TRUE);
  }
  sem_op(shm->sem_id, QUEUE, 1, TRUE);
  sem_op(shm->sem_id, READERS_LOCK, 1, TRUE);
  // debug("POST shm_read ptr: %p sem_id: %d\n", shm, shm->sem_id, READERS);
  // debug("SHM_READ\n");
}

void shm_stop_read(shm* shm) {
  // debug("PRE shm_stop_read ptr: %p sem_id: %d\n", shm, shm->sem_id, READERS);
  sem_op(shm->sem_id, READERS_LOCK, -1, TRUE);
  sem_op(shm->sem_id, READERS, -1, TRUE);
  int readers = sem_get_value(shm->sem_id, READERS);
  if (readers == 0) {
    sem_op(shm->sem_id, WRITE_LOCK, 1, TRUE);
  }
  sem_op(shm->sem_id, READERS_LOCK, 1, TRUE);
  // debug("POST shm_stop_read ptr: %p sem_id: %d\n", shm, shm->sem_id, READERS);
  // debug("SHM_STOP_READ\n");
}

void shm_write(shm* shm) {
  // debug("SHM_QUEUE\n");
  sem_op(shm->sem_id, QUEUE, -1, TRUE);
  sem_op(shm->sem_id, WRITE_LOCK, -1, TRUE);
  sem_op(shm->sem_id, QUEUE, 1, TRUE);
  // debug("SHM_WRITE\n");
}

void shm_stop_write(shm* shm) {
  sem_op(shm->sem_id, WRITE_LOCK, 1, TRUE);
  // debug("SHM_STOP_WRITE\n");
}

void shm_delete(shm* shm) {
  if (shmctl(shm->id, IPC_RMID, NULL) == -1) {
    debug("shmctl\n");
  }
  sem_delete(shm->sem_id);
  free(shm);
}

