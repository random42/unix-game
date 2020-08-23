#ifndef PAWN_H
#define PAWN_H

#include "game.h"

void init();
void start();
void play_round();
square* get_target();
void play();
square* choose_next_square(square* target);
void move_towards(square* target);
void move_to(square* s);
void term();

#endif
