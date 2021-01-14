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

string read_user_login() {
  char id[MAX_ID];
  printf("Please input your login: ");
  scanf("%s", id);
  printf("-----------------------------\n");
  return (string(id));
}

void print_instructions() {
  printf("-----------------------------\n");
  printf("GAME INSTRUCTIONS:");
  printf("To move your piece type the row and the columns number, separated by a space, like: 1 2\n");
}

class RequestBuilder {
  private:
    string player_id;
    string port;
  public:
    RequestBuilder(string player_id, int self_port) {
      this->player_id = player_id;
      this->port = to_string(self_port);
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

int write_socket(string message, int socketfd) {
  char buffer[MAX_BYTES];
  snprintf(buffer, message.size() + 1, "%s", message.c_str());
  return write(socketfd, buffer, MAX_BYTES);
}

// Split the response string using space as separator
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


int main(int argc, char *argv[]) {

  // Validate input
  if (argc != 3) {
    perror("Invalid input! Usage:\n<tcp_port> <upd_port>\n");
  }

  int tcp_listen_port = atoi(argv[1]); // Port used to communicate with TCP server
  int udp_listen_port = atoi(argv[2]); // Listening port to get opponent moves

  // Configure variables to connect with the TCP server
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

  // Request player unique ID through stdin
  printf("\n\n[---WELCOME---]\n");
  string self_player_id =  read_user_login();

  // Create UDP socket to write moves to the opponent
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
  Bind(selffd_udp, (struct sockaddr *)&selfaddr_udp);       // bind the UPD server address
  
  // Configure variables for select command usage
  fd_set readfds;                                           // file descriptors set, shows if the socket is available
  FD_ZERO(&readfds);                                        // for reading

  // Manages all client requests to the server
  RequestBuilder *request_builder = new RequestBuilder(self_player_id, udp_listen_port);

  // Game related variables
  GameState state = GameState::PENDING_LOGIN;
  Game *game = new Game();
  Player *self_player = new Player(self_player_id, '?');

  // Buffers
  string message;
  char buffer[MAX_BYTES];
  vector<string> tokens;

  // Opponent related variables
  Player *opponent_player = new Player("", '?');
  string opponent_addr;
  int opponent_port;
  string opponent_id = "";

  print_instructions();

  while (true) {
    // Configure selector
    FD_ZERO(&readfds);
    FD_SET(server_sockfd_tcp, &readfds); // tcp
    FD_SET(selffd_udp, &readfds);        // udp

    // Make LOGIN: send id to server
    if (state == GameState:: PENDING_LOGIN) {
      int n = write_socket(request_builder->login(udp_listen_port), server_sockfd_tcp);
      if (n == 0) {
        printf("Server disconnected! Loging out ...\n");
        exit(1);
      }
      state = GameState::WAITING_PLAYERS_LIST;
      printf("[LOGIN DONE!]\n");
    }

    // Deal with opponent through UDP connection
    if (FD_ISSET(selffd_udp, &readfds)) {
      if (state == GameState::PLAYING) {
        int message_size = read(selffd_udp, buffer, MAX_BYTES);
        buffer[message_size] = 0;

        // Get opponent move and perform it on the game instance
        tokens = parse_response(string(buffer));
        pair<int,int> position = make_pair(atoi(tokens.at(0).c_str()), atoi(tokens.at(1).c_str()));
        printf("[OPPONENT MOVE]: %d %d\n", position.first, position.second);
        game->make_move(opponent_player, position);
        game->print_board();

        // Get player move
        if (!(game->game_over())) {
          printf("[YOUR MOVE]: ");
          int row, col;
          scanf("%d %d", &row, &col);
          // make move locally
          game->make_move(self_player, make_pair(row, col));
          game->print_board();
          write_socket(request_builder->sendMovement(row, col), peerfd_udp);
          state = GameState::PLAYING;
          printf("END MOVE\n");
        } 
        // Game over
        else {
          // Send the game result to the server
          if (game->getWinner() == NULL) {                    // Draw
            write_socket(request_builder->uploadResult(0), server_sockfd_tcp);
          }
          else if (game->getWinner()->id == self_player_id) { // Win
            write_socket(request_builder->uploadResult(1), server_sockfd_tcp);
          } else {                                           // Loss
            write_socket(request_builder->uploadResult(-1), server_sockfd_tcp);
          }
          // If player wants to know the scores
          printf("To get the scores from the server: input ? and enter. To skip to next match press enter.\n");
          char shouldGetScores;
          scanf("%c", &shouldGetScores);
          if(shouldGetScores == '?') {
            write_socket(request_builder->requestScore(), server_sockfd_tcp);
            state = GameState::WAITING_SCORES;
          } else {
            state = GameState::PENDING_LOGIN;
          }
        }
      }
    }

    // Deal with server: reads server responses
    string buffer_str;
    if (FD_ISSET(server_sockfd_tcp, &readfds)) {
      int message_size = read(server_sockfd_tcp, buffer, MAX_BYTES);

      // Empty message means server disconnected
      if (message_size == 0) {
        printf("Server disconnected! Loging out ...\n");
        exit(1);
      }

      buffer[message_size] = 0; // end of string

      switch (state) {
        // Server sent the available players list
        case GameState::WAITING_PLAYERS_LIST:
          buffer_str = string(buffer);
          // No opponent is available, so wait 5 seconds to make another attempt
          if (buffer_str.size() == 0) {
            printf("[NO OPPONENT AVAILABLE AT THE MOMENT]\n");
            printf("[Retrying in 5 seconds ...]\n");
            sleep(5);
            state = GameState::PENDING_LOGIN;
          }
          // Player inputs the opponent id and send an INVITE request to the server
          else {
            printf("\n[---AVAILABLE OPPONENTS LIST---]\n");
            printf("%s", buffer);
            printf("[TYPE OPPONENT NAME, or press x then enter to skip]: ");
            char opponentName[50];
            scanf("%s", opponentName);
            if (string(opponentName) != "x") {
              opponent_id = string(opponentName);
              printf("you choose: %s\n", opponentName);
              write_socket(request_builder->invite(opponentName, udp_listen_port), server_sockfd_tcp);
            }
            state = GameState::WAITING_ACCEPT;
          }
          break;

        // ACCEPT response: server informs that the opponent has accepted or DENIED the invitation
        case GameState::WAITING_ACCEPT:                                 
          tokens = parse_response(string(buffer));

          // ACCEPT
          if (tokens.at(1) == "yes") {
            printf("[CHALLENGE WAS ACCEPTED BY OPPONENT]\n");
            printf("[PLAYER PIECE: X]\n");

            opponent_player = new Player(opponent_id, 'O');
            opponent_player->symbol = 'O';
            opponent_player->id = opponent_id;

            self_player = new Player(self_player_id, 'X');
            self_player->id = self_player_id;
            self_player->symbol = 'X';

            opponent_addr = tokens.at(2);
            opponent_port = atoi(tokens.at(3).c_str());
            peeraddr_udp.sin_port = htons(opponent_port);
            connect(peerfd_udp, (sockaddr *) &peeraddr_udp, sizeof(peeraddr_udp));

            game = new Game(opponent_player, self_player);
            state = GameState::PLAYING;
          }
          // If the opponent has denied the invitation
          else if (tokens.at(1) == "no") {                                 
            printf("[CHALLENGE WAS DENIED BY OPPONENT]\n");
            write_socket(request_builder->login(udp_listen_port), tcp_listen_port);
            state = GameState::PENDING_LOGIN;
          }
          // The opponent has made an invitation
          else {                                                      
            opponent_id = tokens.at(1);
            printf("Opponent [%s] has made an invitation. Accept? (y/n)\n", opponent_id.c_str());
            char ans;
            scanf("\n%c", &ans);

            // You accepted the invitation
            if (ans == 'y') {
              printf("You accepted the invitation, your symbol is: 'O'\n"); fflush(stdout);
              write_socket(request_builder->accept(opponent_id), server_sockfd_tcp);

              opponent_addr = tokens.at(2);
              opponent_port = atoi(tokens.at(3).c_str());

              opponent_player = new Player(opponent_id, 'X');
              opponent_player->symbol = 'X';
              opponent_player->id = opponent_id;
  
              self_player = new Player(self_player_id, 'O');
              self_player->id = self_player_id;
              self_player->symbol = 'O';

              game = new Game(self_player, opponent_player);

              // Get player move
              printf("[YOUR TURN]: ");fflush(stdout);
              int row = 0, col = 0;
              scanf("%d %d", &row, &col);
              game->make_move(self_player, make_pair(row, col));
              game->print_board();
              peeraddr_udp.sin_port = htons(opponent_port);
              printf("[OPPONENT TURN]: ");fflush(stdout);

              if (connect(peerfd_udp, (sockaddr *) &peeraddr_udp, sizeof(peeraddr_udp)) < 0) {
                printf("ERROR UPD\n");
              }
              write_socket(request_builder->sendMovement(row, col), peerfd_udp);

              state = game->game_over() ? GameState::GAME_OVER : GameState::PLAYING;
            } else {
              write_socket(request_builder->deny(opponent_id), server_sockfd_tcp);
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
          state = GameState::PENDING_LOGIN; // Back to the initial state
          break;
        default:
          break;
      }
    }
  }
}