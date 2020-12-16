# This is the echo client server

* The client connects to the server
* The server sends a command
* The client receives the command from the server
* The client executes this command
* The client sends the result of the command
* The server receives the result of the command
* The server sends another command
* The client receives the command
* The client executes the command
* The client sends the result of the command
* The customer types quit / exit / exit


To compile the files:
* make

To clean the executables:
* make clean

To lauch the server (PORT is the port number you choose):
* ./server <PORT>

To lauch the client (same port as above):
* ./client 127.0.0.1 <PORT>

To insert commands that the server will send to clients:
* Edit the file arq01.in, adding or deleting one command per line

To see the the execution flow in the sdout:
* Set the verbose variable in client and server code to 1

To suppres
* Se the variable verbose to 0 in client and server code