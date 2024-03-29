## Strategia

Si definisce la distanza tra due caselle come il minimo numero di mosse necessarie a una pedina per spostarsi da una all'altra, immaginando che non vi siano pedine nella scacchiera.

Una casella è "controllata" da una pedina se e solo se la distanza tra la pedina e la casella è minore o uguale alla distanza minima tra ogni altra pedina e la casella e la pedina ha abbastanza mosse per raggiungere la casella.

Nella fase di posizionamento delle pedine, ogni giocatore posiziona
la pedina in una casella libera casuale.

All'inizio del round ogni giocatore sceglie casualmente tra due strategie da inviare a ogni pedina: una strategia fa si che la pedina cerchi di raggiungere la bandiera controllata più vicina al centro della scacchiera, mentre la seconda strategia punta alla bandiera controllata più distante dal centro.

Finché il round non è finito, ogni pedina cerca continuamente di fare una mossa verso la bandiera controllata che si addice alla propria strategia. Se non controlla nessuna bandiera resta in attesa *SO_MIN_HOLD_NSEC* nanosecondi in modo che le altre pedine muovano e al ciclo successivo potrebbe controllare una bandiera.

Il percorso più veloce verso una casella consente massimo due possibilità di direzione: nel caso di due direzioni disponibili la pedina cercherà di spostarsi possibilmente verso una casella libera. Il tentativo di spostamento richiederà l'accesso al semaforo in maniera bloccante. Non ci saranno mai casi di deadlock perché se due pedine dovessero "scontrarsi" e andare in direzioni opposte ciò significa che non controllavano la casella in cui si dirigono.

## Flusso e sincronizzazione dei processi

Il processo master inizialmente crea il gioco e ne salva i dati in memoria condivisa. Imposta il proprio id di processo come id del suo gruppo di processi, che verrà ereditato da tutti i processi figli in modo da mandare facilmente segnali a tutti i processi del programma. Dopodiché crea i processi giocatori, i quali creano a sua volte i processi pedine, e inizia la fase di posizionamento delle pedine.

### Fase di posizionamento

Questa fase è sincronizzata tramite il semaforo *SEM_PLACEMENT*. Inizialmente il semaforo ha valore 0. Per iniziare la fase di posizionamento il master imposta *SEM_PLACEMENT* a 1. Ogni giocatore è identificato da un numero *g* che va da 1 a *SO_NUM_G* a lui noto. Una variabile *placement_round* che va da 0 a *SO_NUM_P - 1* identifica il round in cui ogni giocatore posiziona una pedina. Il round di posizionamento per un giocatore *g* è quindi definito da `r = (placement_round * SO_NUM_G) + g`.

Ogni giocatore effettua un'operazione sul semaforo pari a *-r*, posiziona la pedina scrivendo nella memoria condivisa (il cui accesso è sincronizzato) e aumenta il semaforo di *r+1*. Il master aspetta il termine di questa fase effettuando un'operazione sul semaforo pari a `-((SO_NUM_G * SO_NUM_P) + 1)`.

### Fase di gioco

A ogni round il master utilizza i seguenti semafori per sincronizzare i processi: *SEM_ROUND_START*, *SEM_ROUND_READY*. I valori iniziali sono rispettivamente 1 e *SO_NUM_G*.

Il master posiziona le bandiere e imposta *SEM_ROUND_START* a 0. A questo punto i giocatori possono inviare la strategia alle proprie pedine. Per comunicare con le pedine i giocatori utilizzano una coda di messaggi condivisa, dove l'*mtype* del messaggio corrisponde al *pid* della pedina destinataria. Dopo aver mandato tutti i messaggi, ogni giocatore diminuisce *SEM_ROUND_READY* di 1. Quando *SEM_ROUND_READY* vale 0, il round può iniziare: il master fa partire il timer e le pedine iniziano a muoversi. Quando una pedina vuole spostarsi utilizza il set di semafori *SEM_SQUARES* ognuno dei quali è associato a una casella e ha valore 0 nel caso la casella sia occupata e valore 1 se è libera. Una volta effettuato uno spostamento la pedina aggiorna lo stato del gioco nella memoria condivisa. Se una pedina cattura un bandiera invia un messaggio al processo master. Il processo master, in questa fase, ascolta quel segnale tante volte quante il numero di bandiere che ha posizionato, e ogni volta aggiorna il punteggio del gioco. Se tutte le bandiere vengono catturate, il master disattiva il timer, reimposta *SEM_ROUND_START* a 1 e segnala la fine del round tramite il segnale *SIGUSR2*, in modo da sbloccare eventuali attese delle pedine sui semafori. Se invece il timer scatta, allora provvede a terminare i processi tramite il segnale *SIGTERM* e stampare i risultati del gioco.
