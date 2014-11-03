#!/bin/bash

S=60
N=120

HOST=192.168.1.249

while [ 1 ]; do
 mosquitto_pub -h ${HOST} -i talorg-publisher -t stop/1170 -r -m $S
 mosquitto_pub -h ${HOST} -i talorg-publisher -t stop/1170/next -r -m $N
 let S=S-1
 let N=N-1
 if [ $S -eq 0 ]; then
  S=$N
  N=120
 fi
 sleep 1
done
