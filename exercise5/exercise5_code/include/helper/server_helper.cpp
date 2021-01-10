#include <string>
#include <string.h>
#include "server_helper.h"

using namespace std;

void validate_input(int argc, char **argv, char *error)
{
  if (argc != 2)
  {
    strcpy(error, "usage: ");
    strcat(error, argv[0]);
    strcat(error, " <#Port>");
    perror(error);
    exit(1);
  }
}

string get_ip_address(int sockfd, struct sockaddr_in servaddr)
{
  struct sockaddr_in server_socket;
  int server_socket_length = sizeof(servaddr);
  getsockname(sockfd, (sockaddr *)&server_socket, (socklen_t *)&server_socket_length);
  char buffer[INET_ADDRSTRLEN];
  const char *printable_addr = inet_ntop(AF_INET, &server_socket.sin_addr,
                                         buffer, INET_ADDRSTRLEN);
  return string(printable_addr);
}

FILE *create_file(string filepath) {
  FILE *connections_fd = NULL;
  connections_fd = fopen(filepath.c_str(), "w");
  if (connections_fd == NULL)
  {
    perror("Error opening file");
    exit(1);
  }
  return connections_fd;
}