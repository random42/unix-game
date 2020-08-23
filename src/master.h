#ifndef MASTER_H
#define MASTER_H
#include "game.h"

void init();
void start();
void placement_phase();
void play_round();
// piazza le bandiere e ritorna il numero di bandiere
int place_flags();
void wait_flag_captures(int flags);
void term();

#endif
