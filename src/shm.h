#ifndef SHM_H
#define SHM_H

typedef struct shm {
  int id; /* id della memoria */
  int size; /* byte allocati */
  void* ptr; /* puntatore all'inizio della memoria */
  int sem_id; /* id del set di semafori usati per sincronizzare lettura/scrittura */
} shm;

shm* shm_create(int key, int size);
shm* shm_get(int key);
void shm_read(shm* shm);
void shm_stop_read(shm* shm);
void shm_write(shm* shm);
void shm_stop_write(shm* shm);
void shm_delete(shm* shm);

#endif
