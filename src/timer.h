#ifndef TIMER_H
#define TIMER_H

void set_timeout(void (*handler)(int), int sec, int microsec, int atomic);
void clear_timeout();

#endif