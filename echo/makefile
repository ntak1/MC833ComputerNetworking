all: client.o server

server: server.o syscalls.o
	gcc -Wall -o server server.o syscalls.o

server.o: server.c
	gcc -Wall server.c -o server.o -c

syscalls.o: syscalls.c
	gcc -Wall syscalls.c -o syscalls.o -c

client.o: client.c
	gcc -Wall client.c -o client.o

clean:
	rm -rf *.o
	rm server