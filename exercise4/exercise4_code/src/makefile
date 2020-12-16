CC=gcc

CC_FLAGS=-Wall

HELPER_DIR=helper

SYSCALLS_DIR=syscalls

all: client server

# Server
server: server.o syscalls.o server_helper.o
	$(CC) $(CC_FLAGS) -o server server.o server_helper.o syscalls.o

server.o: server.c
	$(CC) $(CC_FLAGS) server.c -o server.o -c

# Dependencies
syscalls.o: $(SYSCALLS_DIR)/syscalls.c
	$(CC) $(CC_FLAGS) $(SYSCALLS_DIR)/syscalls.c -o syscalls.o -c

client_helper.o: $(HELPER_DIR)/client_helper.c
	$(CC) $(CC_FLAGS) $(HELPER_DIR)/client_helper.c -o client_helper.o -c

server_helper.o: $(HELPER_DIR)/server_helper.c
	$(CC) $(CC_FLAGS) $(HELPER_DIR)/server_helper.c -o server_helper.o -c

# Client
client: client.o syscalls.o client_helper.o 
	$(CC) $(CC_FLAGS) -o client client.o syscalls.o client_helper.o

client.o: client.c
	$(CC) $(CC_FLAGS) client.c -o client.o -c

clean:
	rm -rf *.o
	rm server
	rm client