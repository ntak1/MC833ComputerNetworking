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

    string login(int port) {
      return "login: " + player_id + " " + to_string(port);
    }

    string invite(string opponent_id, int port) {
      return "invite: " + player_id + " " + opponent_id + " " + to_string(port);
    }

    string accept(string opponent_id) {
      return "accept: " + player_id + " " + opponent_id; 
    }

    string deny(string opponent_id) {
      return "deny: " + player_id + " " + opponent_id; 
    }

    string uploadResult(int result) {
      return "uploadResult: " + player_id + " " + to_string(result);
    }

    string sendMovement(int row, int col) {
      return to_string(row) + " " + to_string(col);
    }

    string requestScore() {
      return "requestScore: " + player_id;
    }
};

int writeSocket(string message, int socketfd) {
  char buffer[MAX_BYTES];
  snprintf(buffer, message.size() + 1, "%s", message.c_str());
  return write(socketfd, buffer, MAX_BYTES);
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

// Get the opponent id from the request
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

int main(int argc, char *argv[]) {

  if (argc != 3) {
    perror("Invalid input! Usage:\n<tcp_port> <upd_port>\n");
  }
  int tcp_listen_port = atoi(argv[1]);
  int udp_listen_port = atoi(argv[2]);

  // Configure variables to connect with the server
  int server_sockfd_tcp;
  struct sockaddr_in server_addr_tcp;

  bzero(&server_addr_tcp, sizeof(server_addr_tcp));
  server_addr_tcp.sin_family = AF_INET;
  server_addr_tcp.sin_port = htons(tcp_listen_port);
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

  // Create UDP socket to write to the opponent
  struct sockaddr_in peeraddr_udp;                          // opponent address
  int peerfd_udp = Socket(AF_INET, SOCK_DGRAM, 0);
  
  peeraddr_udp.sin_family = AF_INET;                        // IPV4
  peeraddr_udp.sin_addr.s_addr = INADDR_ANY;

  // Configure listening socket to read from the opponent
  struct sockaddr_in selfaddr_udp;                          // opponent address
  int selffd_udp = Socket(AF_INET, SOCK_DGRAM, 0);
  selfaddr_udp.sin_family = AF_INET;                        // IPV4
  selfaddr_udp.sin_addr.s_addr = INADDR_ANY; 
  selfaddr_udp.sin_port = htons(udp_listen_port);
  Bind(selffd_udp, (struct sockaddr *)&peeraddr_udp);
  
  print_local_address(peerfd_udp, peeraddr_udp);

  // Configure variables for select command usage
  fd_set readfds;                                                       // file descriptors set
  bool stdio_eof = false;                                               // if the stdio reached EOF
  FD_ZERO(&readfds);

  RequestBuilder *request_builder = new RequestBuilder(self_player_id); // Game related variables
  GameState state = GameState::PENDING_LOGIN;
  Game *game = new Game();
  Player *self_player = new Player(self_player_id, '?');

  string message;                                                       // Buffers
  char buffer[MAX_BYTES];

  Player *opponent_player = new Player("", '?');                        // Opponent related variables
  string opponent_addr;
  int opponent_port;
  string opponent_id = "";

  vector<string> tokens;

  printInstructions();

  while (true) {
    // Configure selector
    FD_ZERO(&readfds);
    if (stdio_eof == false) {            // stdin
      FD_SET(fileno(stdin), &readfds);
    } 
    FD_SET(server_sockfd_tcp, &readfds); // tcp
    FD_SET(selffd_udp, &readfds);        // udp

    if (state == GameState:: PENDING_LOGIN) { // Send player id to server
      int n = writeSocket(request_builder->login(tcp_listen_port), server_sockfd_tcp);
      if (n == 0) {
        printf("Server disconnected! Loging out ...\n");
        exit(1);
      }
      game = new Game();
      state = GameState::WAITING_PLAYERS_LIST;
      printf("[LOGIN DONE!]\n");
    }

    // Deal with opponent
    if (FD_ISSET(selffd_udp, &readfds)) {                               // Receive movement from opponent
      if (state == GameState::PLAYING) {
        int message_size = read(selffd_udp, buffer, MAX_BYTES);
        buffer[message_size] = 0;
        printf("OPPONENT SENT MOVE [%s]\n", buffer);

        // Get opponent move
        pair<int,int> position = getRowColFromCString(buffer, message_size);
        printf("[OPPONENT MOVE]: %d %d\n", position.first, position.second);
        game->make_move(opponent_player, position);
        game->print_board();
        if (!(game->game_over())) {
          // Get player move
          printf("[YOUR MOVE]: ");
          int row, col;
          scanf("%d %d", &row, &col);
          game->make_move(self_player, make_pair(row, col));
          connect(peerfd_udp, (sockaddr *) &peeraddr_udp, sizeof(peeraddr_udp));
          writeSocket(request_builder->sendMovement(row, col), peerfd_udp);
        } 
        // Game over
        else {
          // Send the game result to the server
          if (game->getWinner() == NULL) {
            writeSocket(request_builder->uploadResult(0), server_sockfd_tcp);
          }
          else if (game->getWinner()->id == self_player_id) {
            writeSocket(request_builder->uploadResult(1), server_sockfd_tcp);
          } else {
            writeSocket(request_builder->uploadResult(-1), server_sockfd_tcp);
          }
          // If player wants to know the scores
          printf("To get the scores from the server: input ? and enter. To skip to next match press enter.\n");
          char shouldGetScores;
          scanf("%c", &shouldGetScores);
          if(shouldGetScores == '?') {
            writeSocket(request_builder->requestScore(), server_sockfd_tcp);
            state = GameState::WAITING_SCORES;
          } else {
            state = GameState::PENDING_LOGIN;
          }
        }
      }
    }

    string buffer_str;
    // Deal with server: reads server responses
    int message_size;
    if (FD_ISSET(server_sockfd_tcp, &readfds)) {
      message_size = read(server_sockfd_tcp, buffer, MAX_BYTES);
      if (message_size == 0) {
        printf("Server disconnected! Loging out ...\n");
        exit(1);
      }
      buffer[message_size] = 0;
      switch (state) {
        case GameState::WAITING_PLAYERS_LIST:                           // Server sent the available players list
          buffer_str = string(buffer);
          if (buffer_str.size() == 0) {
            printf("[NO OPPONENT AVAILABLE AT THE MOMENT]\n");
            printf("[Retrying in 5 seconds ...]\n");
            sleep(5);
            state = GameState::PENDING_LOGIN;
          } else {
            printf("\n[---AVAILABLE OPPONENTS LIST---]\n");
            printf("%s", buffer);
            printf("[TYPE OPPONENT NAME, or press x then enter to skip]: ");
            char opponentName[50];
            scanf("%s", opponentName);
            if (string(opponentName) != "x") {
              opponent_id = string(opponentName);
              printf("you choose: %s\n", opponentName);
              writeSocket(request_builder->invite(opponentName, tcp_listen_port), server_sockfd_tcp);                 // Send the invitation
            }
            state = GameState::WAITING_ACCEPT;
          }
          break;

        case GameState::WAITING_ACCEPT:                                 // If the opponent has accepted the invitation
          tokens = parseRequest(string(buffer));
          // printf("DEBUG: [%s] [%s] [%s]\n", tokens.at(0).c_str(), tokens.at(1).c_str(), tokens.at(2).c_str());
          printf("DEBUG [%s]\n", buffer);
          if (tokens.at(1) == "yes") {
            printf("[CHALLENGE WAS ACCEPTED BY OPPONENT]\n");
            printf("[PLAYER PIECE: X]\n");
            state = GameState::PLAYING;
            opponent_player = new Player(opponent_id, 'X');
            opponent_addr = tokens.at(2);
            opponent_port = atoi(tokens.at(3).c_str());
            
            // Peer will be the server 
            peeraddr_udp.sin_port = opponent_port;
            //Bind(peerfd_udp, (struct sockaddr *)&peeraddr_udp);
            // sendto(peeraddr_udp, (const char *), strlen(hello), 
            //     MSG_CONFIRM, (const struct sockaddr *) &servaddr,  
            //         sizeof(servaddr));

            game = new Game(self_player, opponent_player);
            state = GameState::PLAYING;
          } else if (tokens.at(1) == "no") {                                 // If the opponent has denied the invitation
            printf("[CHALLENGE WAS DENIED BY OPPONENT]\n");
            writeSocket(request_builder->login(udp_listen_port), tcp_listen_port);
            state = GameState::WAITING_PLAYERS_LIST;
          } else {                                                      // The opponent has made and invitation
            opponent_id = tokens.at(1);
            printf("Opponent [%s] has made an invitation. Accept? (y/n)\n", opponent_id.c_str());
            char ans;
            scanf("\n%c", &ans);
            printf("DEBUG ans[%c]\n", ans);
            if (ans == 'y') {
              printf("You accepted the invitation, your symbol is: 'O'\n"); fflush(stdout);
              writeSocket(request_builder->accept(opponent_id), server_sockfd_tcp);
              opponent_addr = tokens.at(2);
              opponent_port = atoi(tokens.at(3).c_str());
              opponent_player = new Player(opponent_id, 'X');
              self_player = new Player(self_player_id, 'O');
              self_player->id = self_player_id;
              self_player->symbol = 'O';
              opponent_player->symbol = 'X';
              opponent_player->id = opponent_id;
              game = new Game(self_player, opponent_player);

              // Get player move
              printf("[YOUR MOVE]: ");fflush(stdout);
              int row = 0, col = 0;
              //game->print_board();
              scanf("%d %d", &row, &col);
              game->make_move(self_player, make_pair(row, col));
              game->print_board();
              peeraddr_udp.sin_port = htons(opponent_port);
              //Bind(peerfd_udp, (struct sockaddr *)&peeraddr_udp);
              connect(peerfd_udp, (sockaddr *) &peeraddr_udp, sizeof(peeraddr_udp));
              writeSocket(request_builder->sendMovement(row, col), peerfd_udp);
              state = GameState::PLAYING;
            } else {
              writeSocket(request_builder->deny(opponent_id), server_sockfd_tcp);
            }
          }
          break;
        // The server has sent the scores
        case GameState::WAITING_SCORES:           
          message_size = read(server_sockfd_tcp, buffer, MAX_BYTES);
          buffer[message_size] = 0;
          message = string(buffer);
          printf("\n[---SCORES---]\n");
          printf("%s", message.c_str());
          state = GameState::PENDING_LOGIN;                             // Back to the initial state
          break;
        default:
          break;
      }
    }
  }
}