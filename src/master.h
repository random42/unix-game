#ifndef MASTER_H
#define MASTER_H
#include "game.h"

void init();
void start();
void play_round();
void place_flags();
void wait_players();
void wait_flag_captures();
void end_round();
void end_game();

#endif
