CC=g++
CC_FLAGS=-Wall

# Directories
HASH_GAME_DIR="../include/hash_game"
HELPER_DIR="../include/helper"
SRC_DIR="."
BIN_DIR="../bin"

# Main targets
all: client.o server.o

client.o: client_helper.o
	$(CC) $(CC_FLAGS) $(SRC_DIR)/client.cpp -o $(BIN_DIR)/client.o $(BIN_DIR)/client_helper.o

server.o: server_helper.o
	$(CC) $(CC_FLAGS) $(SRC_DIR)/server.cpp -o $(BIN_DIR)/server.o $(BIN_DIR)/server_helper.o

# Helpers
client_helper.o:
	$(CC) $(CC_FLAGS) $(HELPER_DIR)/client_helper.cpp -o $(BIN_DIR)/client_helper.o -c

server_helper.o:
	$(CC) $(CC_FLAGS) $(HELPER_DIR)/server_helper.cpp -o $(BIN_DIR)/server_helper.o -c

clean:
	rm $(BIN_DIR)/*.o