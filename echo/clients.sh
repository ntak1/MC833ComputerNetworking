#!/bin/bash

# Start 10 clients
./server 5000 &
for counter in $(seq 1 10)
 do `./client "127.0.0.1" "5000" &`;
done

netstat -np TCP | grep "tcp" > out.txt
echo "--------------------------------------"
sleep 4s

netstat -np TCP | grep "tcp" >> out.txt
sleep 4s

echo "--------------------------------------"
netstat -np TCP | grep "tcp" >> out.txt
sleep 4s

echo "--------------------------------------"
netstat -np TCP | grep "tcp" >> out.txt