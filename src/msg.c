#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "msg.h"
#include "debug.h"

static long msg_size = sizeof(message) - sizeof(long);

int msg_init(int key) {
  int id = msgget(key, IPC_CREAT | 0600);
  if (id == -1) {
    error("msg_init\n");
  }
  return id;
}

void msg_send(int id, message* m, int wait) {
  int flag = wait ? 0 : IPC_NOWAIT;
  int r = msgsnd(id, m, msg_size, flag);
  if (r == -1) {
    error("msgsnd\n");
  }
}

int msg_receive(int id, message* buffer, int wait) {
  int flag = wait ? 0 : IPC_NOWAIT;
  int r = msgrcv(id, buffer, msg_size, getpid(), flag);
  if (r == -1) {
    if (!wait && errno == ENOMSG)
      return -1;
    else {
      error("msgrcv\n");
    }
  }
  return 0;
}

void msg_close(int id) {
  int a = msgctl(id, IPC_RMID, NULL);
  if (a == -1) {
    debug("msgctl\n");
  }
}
