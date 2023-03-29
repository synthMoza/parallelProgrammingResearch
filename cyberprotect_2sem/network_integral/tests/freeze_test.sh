#!/bin/bash

CYAN="\e[36m"
ENDCOLOR="\e[0m"

SERVER_EXEC=server
CLIENT_EXEC=client

a=1
b=50000000

clientsNumber=5
clientsToFreeze=(2 1)
clientsThreads=(1 1 1 1 1)
clientPids=()
port=8762

echo -e "${CYAN} TEST START ${ENDCOLOR}"
for t in ${clientsThreads[@]}; do
    ./$CLIENT_EXEC $t &
    clientPid=$!
    clientPids+=($clientPid)
done

./$SERVER_EXEC $clientsNumber $a $b &
serverPid=$!

sleep 2s
for idx in ${clientsToFreeze[@]}; do
    sleep 1s
    echo -e "${CYAN} FREEZING CLIENT $idx ${ENDCOLOR}"
    kill -STOP ${clientPids[$idx]}
done

sleep 5s
iptables -A INPUT -p tcp --dport $port -j DROP

for idx in ${clientsToFreeze[@]}; do
    echo -e "${CYAN} UNFREEZING CLIENT $idx ${ENDCOLOR}"
    kill -CONT ${clientPids[$idx]}
done

sleep 10s

iptables -D INPUT -p tcp --dport $port -j DROP
tail --pid=$serverPid -f /dev/null

echo -e "${CYAN} TEST END ${ENDCOLOR}"
