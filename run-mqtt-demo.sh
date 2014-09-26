#!/bin/bash

S=60
N=120

while [ 1 ]; do
 mosquitto_pub -i demo-published -t stop/1170 -r -m $S
 mosquitto_pub -i demo-published -t stop/1170/next -r -m $N
 sleep 1
 let S=S-1
 let N=N-1
 if [ $S -eq 0 ]; then
  S=$N
  N=120
 fi
done
