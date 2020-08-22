#ifndef MSG_H
#define MSG_H

typedef struct message {
  // pid of receiver
  long mtype;
  // tells pawn if has to move
  int move;
} message;

int msg_init(int key);
void msg_send(int id, message* m, int wait);
int msg_receive(int id, message* buffer, int wait);
void msg_close(int id);

#endif
