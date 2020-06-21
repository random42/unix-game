#ifndef SHM_H
#define SHM_H

typedef struct shm {
  int id;
  int size; // memory size in bytes
  void* ptr; // memory pointer
  void* free; // free memory
  int sem_id; // semaphore id
} shm;

shm* shm_create(int key, int size);
shm* shm_get();
int shm_get_free_space(shm* shm);
// allocate memory, only one process can use it
void* shm_alloc(shm* shm, int bytes);
void shm_read(shm* shm);
void shm_stop_read(shm* shm);
void shm_write(shm* shm);
void shm_stop_write(shm* shm);
void shm_delete(shm* shm);

#endif
