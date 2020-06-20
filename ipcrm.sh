#!/bin/bash

# script trovato su internet per rimuovere tutte le strutture IPC
# in caso di errori

for i in $(ipcs -a | grep "^s" | awk '{ print $2 }'); do
  ipcrm -s $i
done

for i in $(ipcs -a | grep "^m" | awk '{ print $2 }'); do
  ipcrm -m $i
done

for i in $(ipcs -a | grep "^q" | awk '{ print $2 }'); do
  ipcrm -q $i
done
