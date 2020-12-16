CC=gcc

CC_FLAGS=-Wall

all: client server

server: server.o syscalls.o
	$(CC) $(CC_FLAGS) -o server server.o syscalls.o

server.o: server.c
	$(CC) $(CC_FLAGS) server.c -o server.o -c

syscalls.o: syscalls.c
	$(CC) $(CC_FLAGS) syscalls.c -o syscalls.o -c

client: client.o syscalls.o
	$(CC) $(CC_FLAGS) -o client client.o syscalls.o

client.o: client.c
	$(CC) $(CC_FLAGS) client.c -o client.o -c

clean:
	rm -rf *.o
	rm server
	rm client