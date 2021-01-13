#!/bin/bash
make

# Launch the server
#../bin/server.o 5000

tcp_port=5010
upd_port=5501
# Launch the clients
../bin/client.o $tcp_port $upd_port