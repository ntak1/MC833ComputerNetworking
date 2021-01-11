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
#define DEFAULT_UDP_SERVER_PORT 5500

string readUserLogin() {
  char id[MAX_ID];
  printf("Please input your login: ");
  scanf("%s", id);
  printf("-----------------------------\n");
  return (string(id));
}

int main(void) {
  string self_player_id =  readUserLogin();

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
  printf("User: %s connected to the SERVER!\n", self_player_id.c_str());

  // Create UDP socket to communicate with opponent player
  struct sockaddr_in peeraddr_udp; // opponent address
  int peerfd_udp = Socket(AF_INET, SOCK_DGRAM, 0);
  
  peeraddr_udp.sin_family = AF_INET; // IPV4
  peeraddr_udp.sin_addr.s_addr = INADDR_ANY;
  peeraddr_udp.sin_port = htons(DEFAULT_UDP_SERVER_PORT);

  Bind(peerfd_udp, (struct sockaddr *)&peeraddr_udp);

  // Configure variables for select command usage
  fd_set readfds;           // file descriptors set
  bool stdio_eof = false;   // if the stdio reached EOF

  FD_ZERO(&readfds);

  while (true) {
    // Connect to the server and get the opponent address

    // Configure UDP socket to play the game with opponent
    if (stdio_eof == false) {            // stdin
      FD_SET(fileno(stdin), &readfds);
    } 
    FD_SET(server_sockfd_tcp, &readfds); // tcp
    FD_SET(peerfd_udp, &readfds);        // udp

    if (FD_ISSET(fileno(stdin), &readfds)) {
      // Deal with stdin
    }

    if (FD_ISSET(peerfd_udp, &readfds)) {
      // Deal with opponent
    }

    if (FD_ISSET(server_sockfd_tcp, &readfds)) {
      // Deal with server
    }
  }
}