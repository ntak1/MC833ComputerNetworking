#!/bin/bash
port=5200

echo -n "[Compiling client and server] "
make

echo -n "[Cleaning output files] "
rm ../output_files/out.txt

echo -n "[Starting server on port $port] "
./server $port &

sleep 2

echo -n "[Starting client] "
./client 127.0.0.1 $port < ../input_files/DomCasmurro.txt > ../output_files/out.txt
# ./client 127.0.0.1 $port < ../input_files/small.txt


make clean