#ifndef PAWN_H
#define PAWN_H

#include "game.h"

void init();
void start();
void play_round();
square* get_target();
void play();
Direction choose_direction(square* target);
void move(square* square);
void wait_round_end();
void term();

#endif
