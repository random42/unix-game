#ifndef SIG_H
#define SIG_H

void set_signal_handler(int signal, void (*f)(int), int atomic);
void send_sig(int pid, int sig);
void wait_sig(int sig);

#endif