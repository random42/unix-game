#ifndef SEM_H
#define SEM_H


int sem_get(int key);
int sem_create(int key, int n_sems);
void sem_delete(int sem_id);
int sem_op(int sem_id, int sem_num, short op, int wait);
void sem_set(int sem_id, int sem_num, int val);
int sem_get_value(int sem_id, int sem_num);

#endif
