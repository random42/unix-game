#ifndef MSG_H
#define MSG_H

typedef struct message {
  long mtype; // pid del processo ricevente
  // vale 0 se la strategia è concentrarsi sulle bandiere più centrali
  // o 1 per le bandiere più esterne
  int strategy;
} message;

int msg_init(int key);
void msg_send(int id, message* m, int wait);
int msg_receive(int id, message* buffer, int wait);
void msg_close(int id);

#endif
