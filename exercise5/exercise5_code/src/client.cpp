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

    string sendMovement(int row, int col) {
      return to_string(row) + " " + to_string(col);
    }
};

int writeSocket(string message, int socketfd) {
  char buffer[MAX_BYTES];
  snprintf(buffer, message.size(), "%s", message.c_str());
  return write(socketfd, buffer, message.size());
}

pair<int, int> getRowColFromCString(char message[], int message_size) {
  int row, col;
  char temp[50];
  int j = 0;
  for (int i = 0; i < message_size; i++) {
    if (message[i] == ' ') {
      temp[j] = 0;
      row = atoi(temp);
      j = 0;
    } else {
      temp[j] = message[i];
      j++;
    }
  }
  temp[j] = 0;
  col = atoi(temp);
  return make_pair(row, col);
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

  RequestBuilder *request_builder = new RequestBuilder(self_player_id);
  GameState state = GameState::PENDING_LOGIN;
  string opponent_id = "";
  string message;
  char buffer[MAX_BYTES];
  Game *game = new Game();
  Player *self_player = new Player(self_player_id, '?');
  Player *opponent_player = new Player("", '?');

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
      int n = writeSocket(request_builder->login(), server_sockfd_tcp);
      if (n == 0) { exit(1);}
      game = new Game();
      state = GameState::WAITING_PLAYERS_LIST;
      break;
    }

    // Deal with stdin
    if (FD_ISSET(fileno(stdin), &readfds)) {
      switch (state) {
        case GameState::WAITING_NEW_PLAYER:                             // Player select the opponent
          char opponentName[50];
          scanf("%s", opponentName);
          opponent_id = string(opponentName);
          writeSocket(opponent_id, server_sockfd_tcp);
          state = GameState::WAITING_ACCEPT;
          break;

        case GameState::PLAYER_TURN:
          int row, col;
          scanf("%d %d", &row, &col);
          game->make_move(self_player, make_pair(row, col));
          writeSocket(request_builder->sendMovement(row, col), peerfd_udp);
          if (game->game_over()) {
            state = GameState::GAME_OVER;
          } else {
            state = GameState::WAITING_OTHER_PLAYER_MOVE;
          }
          break;

        case GameState::GAME_OVER:                                      // Game has finished, but player wants to know the scores
          printf("To get the scores from the server: input ? and enter. To skip to next match press enter.\n");
          char shouldGetScores;
          scanf("%c", &shouldGetScores);
          if(shouldGetScores == '?') {
            state = GameState::WAITING_SCORES;
          } else {
            state = GameState::PENDING_LOGIN;
          }
          break;
        default:
          break;
      }
    }

    // Deal with opponent
    if (FD_ISSET(peerfd_udp, &readfds)) {                               // Receive movement from opponent
      if (state == GameState::WAITING_OTHER_PLAYER_MOVE) {
        int message_size = read(peerfd_udp, buffer, MAX_BYTES);
        buffer[message_size] = 0;
        pair<int,int> position = getRowColFromCString(buffer, message_size);
        printf("Opponent move: %d %d\n", position.first, position.second);
        game->make_move(opponent_player, position);
        state = GameState::PLAYER_TURN;
      }
    }

    // Deal with server
    int message_size;
    if (FD_ISSET(server_sockfd_tcp, &readfds)) {
      switch (state) {
        case GameState::WAITING_PLAYERS_LIST:                           // Server sent the available players list
          message_size = read(server_sockfd_tcp, buffer, MAX_BYTES);
          buffer[message_size] = 0;
          printf("%s",buffer);
          state = GameState::WAITING_NEW_PLAYER;
          break;

        case GameState::WAITING_ACCEPT:                                 // If the opponent has accepted the invitation
          message_size = read(server_sockfd_tcp, buffer, MAX_BYTES);
          if (message_size == 0) {exit(1);}
          buffer[message_size] = 0;
          message = string(buffer);
          if (message == "yes") {
            state = GameState::PLAYER_TURN;
            opponent_player = new Player(opponent_id, 'X');
            game = new Game(self_player, opponent_player);
          } else if (message == "no") {
            state = GameState::WAITING_PLAYERS_LIST;
          } else {                                                      // The opponent has made and invitation

          }
          break;

        case GameState::GAME_OVER:                                      // The game has finished, send the result to the server
          if (game->getWinner() == NULL) {
            writeSocket(request_builder->uploadResult(0), server_sockfd_tcp);
          }
          else if (game->getWinner()->id == self_player_id) {
            writeSocket(request_builder->uploadResult(1), server_sockfd_tcp);
          } else {
            writeSocket(request_builder->uploadResult(-1), server_sockfd_tcp);
          }
          break;

        case GameState::WAITING_SCORES:                                 // The server has sent the scores
          message_size = read(server_sockfd_tcp, buffer, MAX_BYTES);
          buffer[message_size] = 0;
          message = string(buffer);
          printf("%s", message.c_str());
          state = GameState::PENDING_LOGIN;                             // Back to the initial state
          break;

        default:
          break;
      }
    }
  }
}