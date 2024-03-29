#ifndef CONF_H
#define CONF_H

#define SHM_KEY 1234
#define SEM_DEBUG_KEY 43243
#define SEM_GAME_KEY 12344
#define SEM_SQUARES_KEY 48374
#define MSG_KEY 23442

#define SEM_GAME_N 3
#define SEM_PLACEMENT 0
#define SEM_ROUND_READY 1
#define SEM_ROUND_START 2

#include <signal.h>
#define ROUND_END_SIGNAL SIGUSR1

#define CENTER_STRATEGY 0
#define EXTERN_STRATEGY 1

#endif
