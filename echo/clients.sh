#!/bin/bash

# Start 10 clients
for i in $(seq 1 10) do
  ./client 127.0.0.1 5000 &&
done
