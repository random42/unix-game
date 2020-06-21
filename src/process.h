#ifndef PROCESS_H
#define PROCESS_H

int get_process_group_id();
void set_process_group_id(int pid, int pgid);
void set_signal_handler(int signal, void (*f)(int), int atomic);
void send_sig(int pid, int sig);
void wait_sig(int sig);

#endif