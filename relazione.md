# Strategia

Si definisce la distanza tra due caselle come il minimo numero di mosse necessarie a una pedina per spostarsi da una all'altra, immaginando che non vi siano pedine nella scacchiera.

Una casella è "controllata" da una pedina se e solo se la
distanza tra la pedina e la casella è minore o uguale alla distanza
minima tra ogni altra pedina e la casella e la pedina ha abbastanza mosse per raggiungere la casella.

## Posizionamento pedine

Nella fase di posizionamento delle pedine, ogni giocatore posiziona
la pedina nella casella da cui può controllare il maggior numero di caselle.
In caso due o più caselle di posizionamento forniscano alla pedina lo stesso numero di caselle controllate, si cerca di minimizzare la somma delle distanze tra la pedina e ogni casella controllata.

Questa strategia fa sì che ogni pedina venga posizionata al
centro dell'area più libera della scacchiera.

## Istruzioni del giocatore

All'inizio del round ogni giocatore verifica per ogni bandiera
le pedine che la controllano e per ogni sua pedina segna le
bandiere controllate. Se una bandiera è controllata solamente
da pedine proprie, allora quella bandiera sarà l'obiettivo
della pedina che controlla meno bandiere. Dopodiché a ogni pedina
senza obiettivo assegna la bandiera controllata con punteggio
maggiore che non è stata assegnata ad altre pedine.

In questo modo ci si assicura che le bandiere controllate da un solo giocatore
vengano sempre conquistate, mentre le bandiere contestate da più giocatori
verranno valutate in base al punteggio.

## Mosse della pedina

Ogni pedina riceve dal proprio giocatore le coordinate della bandiera
che deve cercare di catturare, oppure le proprie coordinate nel caso
non controlli bandiere. Il percorso più veloce verso una casella
consente massimo due possibilità di direzione: nel caso di due direzioni
disponibili la pedina cercherà di spostarsi possibilmente verso una
casella libera. Il tentativo di spostamento richiederà l'accesso al semaforo
in maniera bloccante. Non ci saranno mai casi di deadlock perché se
due pedine dovessero "scontrarsi" e andare in direzioni opposte 
ciò significa che non controllavano la casella in cui si dirigono.

# Flusso e sincronizzazione dei processi

Il processo master inizialmente crea il gioco e ne salva i dati
in memoria condivisa. Importa il proprio id di processo come id del suo gruppo di processi, che verrà ereditato da tutti i processi figli in modo da mandare facilmente segnali a tutti i processi del programma. Dopodiché crea i processi giocatori, i quali creano a sua volte i processi pedine, e inizia la fase di posizionamento delle pedine.

## Fase di posizionamento

Questa fase è sincronizzata tramite il semaforo *SEM_PLACEMENT*. Per iniziare il posizionamento il master imposta *SEM_PLACEMENT* a 0. Ogni giocatore è identificato da un numero *g* che va da 0 a *SO_NUM_G* a lui noto. Una variabile *placement_round* che va da 0 a *SO_NUM_P* identifica il round in cui ogni giocatore posiziona una pedina. Il round di posizionamento per un giocatore *g* è quindi definito da `r = (placement_round * SO_NUM_G) + g`.

Ogni giocatore effettua un'operazione sul semaforo pari a *-r*, posiziona la pedina scrivendo nella memoria condivisa (il cui accesso è sincronizzato) e aumenta il semaforo di *r+1*. Il master aspetta il termine di questa fase effettuando un'operazione sul semaforo pari a `-(SO_NUM_G * SO_NUM_P)`.

## Fase di gioco

A ogni round il master utilizza i seguenti semafori per sincronizzare i processi: *SEM_ROUND_START*, *SEM_ROUND_READY*. I valori iniziali sono rispettivamente 1 e *SO_NUM_G*.

Il master posiziona le bandiere e imposta *SEM_ROUND_START* a 0. A questo punto i giocatori possono inviare la strategia alle proprie pedine. Per comunicare la strategia (casella target) i giocatori utilizzano una coda di messaggi condivisa, dove l'*mtype* del messaggio corrisponde al *pid* della pedina destinataria. Dopo aver mandato tutti i messaggi, ogni giocatore diminuisce *SEM_ROUND_READY* di 1. Quando *SEM_ROUND_READY* vale 0, il round può iniziare: il master fa partire il timer e le pedine iniziano a muoversi. Quando una pedina vuole spostarsi utilizza il set di semafori *SEM_SQUARES* ognuno dei quali è associato a una casella e ha valore 0 nel caso la casella sia occupata e valore 1 se è libera. Una volta effettuato uno spostamento la pedina aggiorna lo stato del gioco nella memoria condivisa. Se una pedina cattura un bandiera invia il segnale *SIGUSR1* a tutto il gruppo di processi. Il processo master, in questa fase, ascolta quel segnale tante volte quante il numero di bandiere che ha posizionato, e ogni volta aggiorna il punteggio del gioco. Se tutte le bandiere vengono catturate, il master disattiva il timer, reimposta *SEM_ROUND_START* a 1 e segnala la fine del round tramite il segnale *SIGUSR2*, in modo da sbloccare eventuali attese delle pedine sui semafori. Se invece il timer scatta, allora provvede a terminare i processi tramite il segnale *SIGTERM* e stampare i risultati del gioco.