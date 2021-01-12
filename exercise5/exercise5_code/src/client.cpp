/**
 * MC833
 * Author: Naomi Takemoto
 * Jan 2021
 * The client-client UDP communication is based on the following tutorial
 * https://www.geeksforgeeks.org/udp-server-client-implementation-c/
 */

#include <cstdio>

#include "../include/syscalls/syscalls.h"
#include "../include/helper/client_helper.h"
#include "../include/hash_game/hash_game.h"

using namespace std;

#define MAX_BYTES 1024
#define MAX_ID 50

#define DEFAULT_SERVER_ID "127.0.0.1"
#define DEFAULT_SERVER_PORT 5000
#define DEFAULT_UDP_SERVER_PORT 5502


string readUserLogin() {
  char id[MAX_ID];
  printf("Please input your login: ");
  scanf("%s", id);
  printf("-----------------------------\n");
  return (string(id));
}

void printInstructions() {
  printf("-----------------------------\n");
  printf("GAME INSTRUCTIONS:");
  printf("To move your piece type the row and the columns number, separated by a space, like: 1 2\n");
}

class RequestBuilder {
  private:
    string player_id;
  public:
    RequestBuilder(string player_id) {
      this->player_id = player_id;
    }

    string login() {
      return "login: " + player_id;
    }

    string invite(string opponent_id) {
      return "invite: " + player_id + " " + opponent_id;
    }

    string accept(string opponent_id) {
      return "accept:" + player_id + " " + opponent_id; 
    }

    string uploadResult(int result) {
      return "uploadResult: " + player_id + " " + to_string(result);
    }

};

int writeSocket(string message, int socketfd) {
  char buffer[MAX_BYTES];
  snprintf(buffer, message.size(), "%s", message.c_str());
  return write(socketfd, buffer, message.size());
}


int main(void) {
  // Configure variables to connect with the server
  int server_sockfd_tcp;
  struct sockaddr_in server_addr_tcp;

  bzero(&server_addr_tcp, sizeof(server_addr_tcp));
  server_addr_tcp.sin_family = AF_INET;
  server_addr_tcp.sin_port = htons(DEFAULT_SERVER_PORT);
  if (inet_pton(AF_INET, DEFAULT_SERVER_ID, &server_addr_tcp.sin_addr) <= 0)
  {
    perror("inet_pton error");
    exit(1);
  }

  // Connect to the server
  server_sockfd_tcp = Socket(AF_INET, SOCK_STREAM, 0);
  Connect(server_sockfd_tcp, (struct sockaddr*) &server_addr_tcp);


  string self_player_id =  readUserLogin();
  printf("User: %s connected to the SERVER!\n", self_player_id.c_str());

  // Create UDP socket to communicate with opponent player
  struct sockaddr_in peeraddr_udp;                          // opponent address
  int peerfd_udp = Socket(AF_INET, SOCK_DGRAM, 0);
  
  peeraddr_udp.sin_family = AF_INET;                        // IPV4
  peeraddr_udp.sin_addr.s_addr = INADDR_ANY;
  peeraddr_udp.sin_port = htons(DEFAULT_UDP_SERVER_PORT);

  Bind(peerfd_udp, (struct sockaddr *)&peeraddr_udp);
  print_local_address(peerfd_udp, peeraddr_udp);

  // Configure variables for select command usage
  fd_set readfds;                                           // file descriptors set
  bool stdio_eof = false;                                   // if the stdio reached EOF
  FD_ZERO(&readfds);

  RequestBuilder *requestBuilder = new RequestBuilder(self_player_id);
  GameState state = GameState::PENDING_LOGIN;
  string opponent_id = "";
  string message;
  char buffer[MAX_BYTES];
  Game *game = new Game();
  Player *selfPlayer = new Player(self_player_id, '?');

  printInstructions();

  while (true) {
    // Configure selector
    if (stdio_eof == false) {            // stdin
      FD_SET(fileno(stdin), &readfds);
    } 
    FD_SET(server_sockfd_tcp, &readfds); // tcp
    FD_SET(peerfd_udp, &readfds);        // udp

    // Make login
    if (state == GameState:: PENDING_LOGIN) { // Send player id to server
      int n = writeSocket(requestBuilder->login(), server_sockfd_tcp);
      if (n == 0) { exit(1);}
      game = new Game();
      state = GameState::WAITING_PLAYERS_LIST;
      break;
    }

    // Deal with stdin
    if (FD_ISSET(fileno(stdin), &readfds)) {
      switch (state) {
        case GameState::WAITING_PLAYERS_LIST:
          char opponentName[50];
          scanf("%s", opponentName);
          opponent_id = string(opponentName);
          state = GameState::WAITING_NEW_PLAYER;
        case GameState::PLAYER_TURN:
          int row, col;
          scanf("%d %d", &row, &col);
          game->make_move(selfPlayer, make_pair(row, col));
        default:
          break;
      }
    }

    // Deal with opponent
    if (FD_ISSET(peerfd_udp, &readfds)) {
      if (state == GameState::WAITING_OTHER_PLAYER_MOVE) {
        // TODO: Finish
        state = GameState::PLAYER_TURN;
      }
    }

    // Deal with server
    int message_size;
    if (FD_ISSET(server_sockfd_tcp, &readfds)) {
      switch (state) {
        case GameState::WAITING_PLAYERS_LIST:
          message_size = read(server_sockfd_tcp, buffer, MAX_BYTES);
          buffer[message_size] = 0;
          printf("%s",buffer);
          state = GameState::WAITING_NEW_PLAYER;
          break;
        case GameState::WAITING_NEW_PLAYER: // Send the chose player to the server
          // Opponent has accepted the invitation
          message_size = read(server_sockfd_tcp, buffer, MAX_BYTES);
          if (message_size == 0) {exit(1);}
          buffer[message_size] = 0;
          message = string(buffer);
          if (message == "yes") {
            state = GameState::PLAYER_TURN;
          } else if (message == "no") {
            state = GameState::WAITING_PLAYERS_LIST;
          }
          break;
        case GameState::GAME_OVER:
          if (game->getWinner() == NULL) {
            writeSocket(requestBuilder->uploadResult(0), server_sockfd_tcp);
          }
          else if (game->getWinner()->id == self_player_id) {
            writeSocket(requestBuilder->uploadResult(1), server_sockfd_tcp);
          } else {
            writeSocket(requestBuilder->uploadResult(-1), server_sockfd_tcp);
          }
          state = GameState::PENDING_LOGIN;
          break;
        case GameState::WAITING_SCORES:
          message_size = read(server_sockfd_tcp, buffer, MAX_BYTES);
          buffer[message_size] = 0;
          message = string(buffer);
          printf("%s", message.c_str());
          break;
        default:
          break;
      }
    }
  }
}