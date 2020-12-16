#!/bin/bash

# This is a helper script to execute experiments
# it starts the server and 10 clients in the background
# then it executes netstat command and save its output to a file
# in case the out.txt file

make
./server 5000 &

filename="zombie_output.txt"
filter="127.0.0.1"

netstat -taulpn | grep $filter > $filename &
for counter in $(seq 1 10)
 do ./client "127.0.0.1" "5000" &
done
sleep 1s

echo "--------------------------------------" >> $filename
netstat -taulpn | grep $filter >> $filename

# Uncomment this to see the kill the processes
#ps
#pkill client
#pkill server