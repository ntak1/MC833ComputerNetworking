/*  MC883
 *  Author: Naomi Takemoto
 *  RA: 184849
 *  Unicamp, Jan 2021
 *  
 *  The select command usage was based in the following tutorial
 *  https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/
 */

#include <unistd.h>
#include <string>
#include <vector>
#include <queue>
#include <map>
#include "../include/syscalls/syscalls.h"
#include "../include/helper/server_helper.h"

using namespace std;

#define PUNCTUATION_FILE "punctuation.csv"
#define MAX_DATA_SIZE 100
#define MAX_IP_LENGTH 20
#define MAX_CLIENTS 10
#define LISTENQ 10
#define MAX_BYTES 1024


inline bool is_valid_descriptor(int fd) {
  return fd > 0;
}

// Return the first available player and set clear the its spot on the list
Player* popAvailblePlayer(vector<Player*> &players) {
  for (int i = 0; i < players.size(); i++) {
    if (players[i] != NULL) {
      Player * temp = players[i];
      players[i] = NULL;
      return temp;
    }
  }
  return NULL;
}

// Add a player as available in the first empty position found
void addAvailablePlayer(vector<Player*> &players, Player *player) {
  for (int i = 0; i < players.size(); i++) {
    if (players[i] == NULL) {
      players[i] = player;
    }
  }
}

void removeAvailablePlayer(vector<Player*> players, Player *player) {
  for (int i = 0; i < players.size(); i++) {
    if (players[i] == player) {
      players[i] = NULL;
    }
  }
}

vector<string> parseRequest(string request) {
  char sep = ' ';
  int init = 0;
  vector<string> ans;
  for (int i = 0; i < request.size(); i++) {
    if(request[i] == sep) {
      ans.push_back(request.substr(init, i - init));
      init = i + 1;
    }
  }
  if (init < request.size()) {
    ans.push_back(request.substr(init, request.size() - init));
  }
  return ans;
}

int main(int argc, char **argv) {
  // Connection variables
  char error[MAX_BYTES] = {0};
  char buffer[MAX_BYTES] = {0};

  // Input: args
  validate_input(argc, argv, error);
  string port_str = string(argv[1]);

  // Create and configure server socket
  sockaddr_in servaddr;
  int master_socket = Socket(AF_INET, SOCK_STREAM, 0);
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(atoi(port_str.c_str()));

  // Bind the socket to localhost #port
  Bind(master_socket, (sockaddr *)&servaddr);

  // Get the port number chosen by the system and show on stdout
  struct sockaddr_in curr_addr;
  unsigned long addr_length = sizeof(servaddr);
  int status = getsockname(master_socket, (sockaddr *)&curr_addr, (socklen_t *)&addr_length);
  if (!status) {
    printf("Server listening on port: %u\n", ntohs(curr_addr.sin_port));
  }
  else if (status != 0) {
    perror("Error defining server port.");
    exit(0);
  }

  // Start listening for connections
  Listen(master_socket, LISTENQ);
  printf("Waiting connection ...\n");
  printf("[Process PID] %u\n", getpid());
  fflush(stdout);

  // Variables for select command
  int client_socket[MAX_CLIENTS];
  for (int i = 0; i < MAX_CLIENTS; i++) {
    client_socket[i] = 0;
  }
  fd_set readfds;

  // Business logic
  Punctuation allPlayers;                            // class to keep track of players in the game and their punctuation
  map <int, Player *>  playerFromPort;               // class to get the player, given the server port number associated with it
  vector <Player *> availablePlayers(MAX_CLIENTS);   // list of player ports that are not currently in a game
  

  while (true) {
    FD_ZERO(&readfds);

    // Add main server socket to set
    FD_SET(master_socket, &readfds);
    int max_sd = master_socket;

    // Add child sockets to set
    for (int i = 0; i < MAX_CLIENTS; i++) {
      int socket_descriptor = client_socket[i];
      if (is_valid_descriptor(socket_descriptor)) {
        FD_SET(socket_descriptor, &readfds);
      }
      max_sd = (socket_descriptor > max_sd) ? socket_descriptor: max_sd;
    }

    // Wait for an activity on one of the sockets, waiting indefinitely since the timeout is NULL
    int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
    if ((activity < 0) && (errno) != EINTR) {
      printf("select error");
    }

    // If something happenend on the master_socket, then it is an incoming connection
    if (FD_ISSET(master_socket, &readfds)) {
      int new_socket = Accept(master_socket, (sockaddr *) &servaddr);
      int port = ntohs(servaddr.sin_port);
      string address = string(inet_ntoa(servaddr.sin_addr));

      printf("New connection, socket fd: %d, IP: %s, port: %d\n",
        new_socket, inet_ntoa(servaddr.sin_addr), port);

      Player *player = new Player();
      player->port = port;
      player->address = address;
      addAvailablePlayer(availablePlayers, player);

      // Add new socket to array of sockets
      for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_socket[0] == 0) {
          client_socket[i] = new_socket;
          printf("Adding new_socket to list of sockets in position %d\n", i);
          break;
        }
      }
    }
    // Else there's a new activity on some other socket
    else {
      for (int i = 0; i < MAX_CLIENTS; i++) {
        // Check if it was closing, and also read the incoming message
        int socket_descriptor = client_socket[i];
        if (FD_ISSET(socket_descriptor, &readfds)) {
          int message_size = read(socket_descriptor, buffer, MAX_BYTES);

          socklen_t len_addr = sizeof(servaddr);
          getpeername(socket_descriptor, (sockaddr *) &servaddr, &len_addr);
          int port = ntohs(servaddr.sin_port);
          string address = string(inet_ntoa(servaddr.sin_addr));

          // Client disconnected: close the connection and clear client_socket
          if (message_size == 0) {
            printf("Client disconnected: [IP: %s], [PORT: %d])\n", address.c_str(), port);
            close(socket_descriptor);
            client_socket[i] = 0;
                      
          }
          // Client sent some valid request to the server
          else {
            buffer[message_size + 1] = 0;
            string message = string(buffer);
            vector<string> tokens = parseRequest(message);
            printf("Tokens received from client: ");
            for(auto m: tokens) {
              printf("[%s] ", m.c_str());
            }
            printf("\n");
            printf("Client request: [%s]\n", buffer);

            // Sends to the client its opponent player address if some is available
            if(tokens.at(0) == "login:") {
              printf("[LOGIN] user %s requested list of available players.\n", tokens.at(1).c_str());
              Player *player = popAvailblePlayer(availablePlayers);
              snprintf(buffer, MAX_BYTES, "%s %d", player->address.c_str(), player->port);
              write(socket_descriptor, buffer, sizeof(buffer));
            }

            
          }
        }
      }// end for
    } // end else
  } // end while
  return (0);
}
