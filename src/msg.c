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

static int msg_size = sizeof(msg) - sizeof(long);

int msg_init(int key) {
  int id = msgget(key, IPC_CREAT | 0600);
  if (id == -1) {
    error("msg_init");
  }
  return id;
}

void msg_send(int id, msg* m, int wait) {
  int flag = wait ? 0 : IPC_NOWAIT;
  int r = msgsnd(id, m, msg_size, flag);
  if (r == -1) {
    error("msgsnd\n");
  }
}

int msg_receive(int id, msg* buffer, int wait) {
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
