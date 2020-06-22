#ifndef PLAYER_H
#define PLAYER_H

#include "game.h"

void init();
void start();
void placement_phase();
void play_round();
square* create_strategy(pawn* pawn);
void wait_round_end();
void term();

#endif
