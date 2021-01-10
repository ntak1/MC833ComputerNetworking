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
#include<map>
#include "../include/syscalls/syscalls.h"
#include "../include/helper/server_helper.h"

using namespace std;

#define PUNCTUATION_FILE "punctuation.csv"
#define MAX_DATA_SIZE 100
#define MAX_IP_LENGTH 20
#define MAX_CLIENTS 10
#define LISTENQ 10
#define MAX_BYTES 4096


inline bool is_valid_descriptor(int fd) {
  return fd > 0;
}

struct PunctuationEntry {
  int wins;
  int losses;
  string address;
  string port;
};
typedef struct PunctuationEntry PunctuationEntry;

class Punctuation {
  private:
    map<string, PunctuationEntry> punctuation;

  public:
    int get_wins(string player_name) {
      return punctuation[player_name].wins;
    }

    int get_losses(string player_name) {
      return punctuation[player_name].losses;
    }

    int get_total_matches(string player_name) {
      return punctuation[player_name].losses + punctuation[player_name].losses;
    }

    string get_address(string player_name) {
      return punctuation[player_name].address;
    }

    string get_port(string player_name) {
      return punctuation[player_name].port;
    }
};

int main(int argc, char **argv) {
  // Connection variables
  char error[MAX_BYTES] = {0};
  char buffer[MAX_BYTES] = {0};
  bool verbose = true;

  // File to store the players score
  FILE *punctuation_fd = create_file(PUNCTUATION_FILE);

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
  if (!status && verbose) {
    printf("Server listening on port: %u\n", ntohs(curr_addr.sin_port));
  }
  else if (status != 0) {
    perror("Error defining server port.");
    exit(0);
  }

  // Start listening for connections
  Listen(master_socket, LISTENQ);
  if (verbose != 0) {
    printf("Waiting connection ...\n");
    printf("[parent] Parent pid %u\n", getpid());
    fflush(stdout);
  }

  // Variables for select command
  int client_socket[MAX_CLIENTS];
  for (int i = 0; i < MAX_CLIENTS; i++) {
    client_socket[i] = 0;
  }
  fd_set readfds;

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

    // wait for an activity on one of the sockets, waiting indefinitely since the timeout is NULL
    int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
    if ((activity < 0) && (errno) != EINTR) {
      printf("select error");
    }

    // If something happenend on the master_socket, then it is an incoming connection
    if (FD_ISSET(master_socket, &readfds)) {
      int new_socket = Accept(master_socket, (sockaddr *) &servaddr);
      printf("New connection, socket fd: %d, if: %s, port: %d\n",
       new_socket, inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port));

      // Sends the client the list of available players
      // TODO
      // ...

      // Add new socket to array of sockets
      for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_socket[0] == 0) {
          client_socket[i] = new_socket;
          printf("Adding new_socket to list of sockets in position %d\n", i);
          break;
        }
      }
    }
    else {
      // Else there's a new activity on some other socket
      for (int i = 0; i < MAX_CLIENTS; i++) {
        int socket_descriptor = client_socket[i];
        
        // Check if it was closing, and also read the incoming message
        if (FD_ISSET(socket_descriptor, &readfds)) {
          int message_size = read(socket_descriptor, buffer, MAX_BYTES);

          // Client disconnected
          if (message_size == 0) {
            // Log
            socklen_t len_addr = sizeof(servaddr);
            getpeername(socket_descriptor, (sockaddr *) &servaddr, &len_addr);
            printf("Client disconnected: [IP: %s], [PORT: %d])\n", inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port));

            // Close the connection and clear the client_socket array
            close(socket_descriptor);
            client_socket[i] = 0;            
          }
          else {
            // TODO
            // See what to do depending on the client message
            // ...
            string message = "Dumb message\n";
            write(socket_descriptor, message.c_str(), message.size());
            fputs(message.c_str(), punctuation_fd);
          }
        }
      }// end for
    } // end else
  } // end while
  return (0);
}
