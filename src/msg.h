#ifndef MSG_H

#define MSG_H

typedef enum {MSG_INVITE,MSG_RESPONSE,MSG_VOTE,MSG_CLOSE_GROUP} msg_t;

typedef struct msg {
  // pid of receiver
  long mtype;
  // pid of sender
  int from;
  // msg type
  msg_t type;
} msg;

int msg_init(int key);
void msg_send(int id, msg* m, int wait);
int msg_receive(int id, msg* buffer, int wait);
void msg_close(int id);

#endif
