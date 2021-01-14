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

vector<string> parse_response(string request) {
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

// Receives a message and a socket descriptor and send the message to the corresponding socket
int write_socket(string message, int socketfd) {
  char buffer[MAX_BYTES];
  snprintf(buffer, message.size() + 1, "%s", message.c_str());
  return write(socketfd, buffer, MAX_BYTES);
}

// Build all the server responses
class RequestBuilder {
  public:
    // Return a string containing all the player's scores
    string scoreTableBuilder(PlayersContainer playersContainer, string player_requester) {
      string result;
      auto players = playersContainer.getPlayers();
      for (auto player: players) {
        if (player.second.available && player.first != player_requester) {
          result += player.second.toString();
          result += "-----------------------------\n";
        }
      }
      return result;
    }
    // Build the request that informs a player that other player (inviter) wants to challenge him
    // for a new match.
    string invite(string inviter, string address, int port) {
      printf("DEBUG: %s\n", inviter.c_str());
      return "invitation: " + inviter + " " + address + " " + to_string(port);
    }

    // Build the request that informs a player that his opponent has accepted or not the challenge and
    // send the player the opponet's address and port for further UDP connection
    string accept(string opponent_address, int opponent_port, bool accept) {
      if (accept) {
        return "accept: yes " + opponent_address + " " + to_string(opponent_port);
      } else {
        return "accept: no " + opponent_address + " " + to_string(opponent_port);
      }
    }
};

int getSocketFromPlayerId(map<int, string> playerIdFromSocket, string player_id) {
  for(auto player: playerIdFromSocket) {
    if (player.second == player_id) {
      return player.first;
    }
  }
  return -1;
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
  PlayersContainer allPlayers;                            // class to keep track of players in the game and their punctuation
  map<int, string> playerIdFromSocket;

  RequestBuilder requestBuilder;

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
        
      // Add new socket to array of sockets
      for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_socket[i] == 0) {
          client_socket[i] = new_socket;
          printf("Adding new_socket to list of sockets in position %d\n", i);
          break;
        }
      }
    }
    for (int i = 0; i < MAX_CLIENTS; i++) {
      // Check if it was closing, and also read the incoming message
      int socket_descriptor = client_socket[i];
      if (FD_ISSET(socket_descriptor, &readfds)) {
        printf("Socket descriptor with socket %d is readable.\n", socket_descriptor);

        int message_size = read(socket_descriptor, buffer, MAX_BYTES);

        socklen_t len_addr = sizeof(servaddr);
        getpeername(socket_descriptor, (sockaddr *) &servaddr, &len_addr);
        string address = string(inet_ntoa(servaddr.sin_addr));

        // Client disconnected: close the connection and clear client_socket
        if (message_size == 0) {
          printf("Client disconnected: [IP: %s])\n", address.c_str());
          close(socket_descriptor);
          client_socket[i] = 0;
        }

        // Client sent some valid request to the server
        else {
          buffer[message_size + 1] = 0;
          string message = string(buffer);
          vector<string> tokens = parse_response(message);

          // Sends to the client its opponent player address if some is available
          string newPlayerId, inviter, invited, buffer_str;

          // Login: make the player available
          if(tokens.at(0) == "login:") {
            printf("[LOGIN] user %s requested list of available players.\n", tokens.at(1).c_str());
            if(allPlayers.getAvailablePlayersIds().size() == 1 && allPlayers.getAvailablePlayersIds()[0] == tokens.at(1)) {
              write_socket("", socket_descriptor);
            } else {
              write_socket(requestBuilder.scoreTableBuilder(allPlayers, tokens.at(1)), socket_descriptor);
            }

            newPlayerId = tokens.at(1);
            allPlayers.insertOrUpdatePlayer(newPlayerId, address, atoi(tokens.at(2).c_str()));
            allPlayers.setPlayerAvailable(newPlayerId, true);
            playerIdFromSocket[socket_descriptor] = newPlayerId;
          }
          // Invitation: sent the opponent the inviter id
          // player_x invite player_y, server need to notify player_y
          else if(tokens.at(0) == "invite:") {
            string inviter, invited;
            // Get socket from player id
            inviter = tokens.at(1);
            invited = tokens.at(2);
            printf("[INVITE] [%s] [%s]\n", inviter.c_str(), invited.c_str());
            int invited_player_socket = getSocketFromPlayerId(playerIdFromSocket, invited);
            if (invited_player_socket != -1) {
              write_socket(requestBuilder.invite(inviter, address, atoi(tokens.at(3).c_str())), invited_player_socket);
            } else {
              printf("Error: could not find socket descriptor!\n");
              for (auto temp: playerIdFromSocket) {
                printf("[%d] [%s]\n", temp.first, temp.second.c_str());
              }
            }
          }
          // Player x (invited) accepted the invitation of player y, server informs y (inviter)
          else if (tokens.at(0) == "accept:") {
            string invited = tokens.at(1);
            string inviter = tokens.at(2);
            int inviter_socket = getSocketFromPlayerId(playerIdFromSocket, inviter);
            Player invitedPlayer = allPlayers.getPlayers()[invited];
            write_socket(requestBuilder.accept(invitedPlayer.address, invitedPlayer.port, true), inviter_socket);
            // Mark both players as unavailable
            allPlayers.setPlayerAvailable(invited, false);
            allPlayers.setPlayerAvailable(inviter, false);
          }
          else if (tokens.at(0) == "deny:") {
            string invited = tokens.at(1);
            string inviter = tokens.at(2);
            int inviter_socket = getSocketFromPlayerId(playerIdFromSocket, inviter);
            Player invitedPlayer = allPlayers.getPlayers()[invited];
            write_socket(requestBuilder.accept(invitedPlayer.address, invitedPlayer.port, false), inviter_socket); 
          }
          else if (tokens.at(0) == "uploadResult:") {
            string player_id = tokens.at(1); 
            if (tokens.at(2) == "0") {
              allPlayers.addDrawCount(player_id);
            } else if (tokens.at(2) == "+1") {
              allPlayers.addWinCount(player_id);
            } else if (tokens.at(2) == "-1") {
              allPlayers.addLossCount(player_id);
            } else {
              printf("Invalid argument in uploadResult request!.\n");
            }
          }
          else if (tokens.at(0) == "requestScore:") {
            write_socket(requestBuilder.scoreTableBuilder(allPlayers, tokens.at(1)), socket_descriptor);
          } else {
            printf("Server received an invalid request!\n[%s]", buffer);
          }
        }
      }
    }// end for
  } // end while
  return (0);
}
