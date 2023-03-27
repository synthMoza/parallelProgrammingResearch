#!/bin/bash

CYAN="\e[36m"
ENDCOLOR="\e[0m"

SERVER_EXEC=server
CLIENT_EXEC=client

a=1
b=50000000

clientsNumber=5
clientsThreads=(1 3 2 2 1)

echo -e "${CYAN} TEST START ${ENDCOLOR}"
for t in ${clientsThreads[@]}; do
    ./$CLIENT_EXEC $t &
done

./$SERVER_EXEC $clientsNumber $a $b
echo -e "${CYAN} TEST END ${ENDCOLOR}"
