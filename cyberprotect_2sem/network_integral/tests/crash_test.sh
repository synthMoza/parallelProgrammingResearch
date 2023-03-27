#!/bin/bash

CYAN="\e[36m"
ENDCOLOR="\e[0m"

SERVER_EXEC=server
CLIENT_EXEC=client

a=1
b=50000000

clientsNumber=5
clientsToKill=(2 1)
clientsThreads=(1 3 2 2 1)
clientPids=()

echo -e "${CYAN} TEST START ${ENDCOLOR}"
for t in ${clientsThreads[@]}; do
    ./$CLIENT_EXEC $t &
    clientPid=$!
    clientPids+=($clientPid)
done

./$SERVER_EXEC $clientsNumber $a $b &
serverPid=$!

for idx in ${clientsToKill[@]}; do
    sleep 1s
    echo -e "${CYAN} KILLING CLIENT $idx ${ENDCOLOR}"
    kill -9 ${clientPids[$idx]}
done

tail --pid=$serverPid -f /dev/null

echo -e "${CYAN} TEST END ${ENDCOLOR}"
