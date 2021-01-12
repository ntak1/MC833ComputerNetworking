// ----------------------------------------------------------------------------
// Helper functions implementation
// ----------------------------------------------------------------------------
#include "client_helper.h"

using namespace std;

// Get and print information about the local socket
void print_local_address(int sockfd, struct sockaddr_in addr)
{
  int server_socket_length = sizeof(addr);
  getsockname(sockfd, (sockaddr *)&addr, (socklen_t *)&server_socket_length);
  char buffer[INET_ADDRSTRLEN];
  const char *printable_addr = inet_ntop(AF_INET, &addr.sin_addr,
                                         buffer, INET_ADDRSTRLEN);
  printf("Local IP Address: %s\n", printable_addr);
  printf("Local Port: %d\n", ntohs(addr.sin_port));
  printf("------------------------------------\n");
}