#!/bin/bash
make

tcp_port=5012
upd_port=5503
# Launch the clients
../bin/client.o $tcp_port $upd_port