#ifndef PROCESS_H
#define PROCESS_H

int get_process_id();
int get_parent_process_id();
int get_process_group_id();
void set_process_group_id(int pid, int pgid);
// esegue un nuovo processo
int fork_and_exec(char* path, char** argv);
// imposta una funzione come handler di un segnale
// atomic e' un booleano per definire se l'esecuzione dell'handler 
// puo' essere interrotta da altri segnali o meno
void set_signal_handler(int signal, void (*f)(int), int atomic);
// manda un segnale
void send_sig(int pid, int sig);
// attende un segnale
void wait_sig(int sig);

#endif