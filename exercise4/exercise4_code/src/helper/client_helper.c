// ----------------------------------------------------------------------------
// Helper functions implementation
// ----------------------------------------------------------------------------
#include "client_helper.h"
// Validate input
void validate_input(int argc, char **argv, char error[])
{
  if (argc != 3)
  {
    strcpy(error, "usage: ");
    strcat(error, argv[0]);
    strcat(error, " <IPaddress>");
    strcat(error, " <#Port>");
    perror(error);
    exit(1);
  }
}

// Send the command result back to the server
void send_command_result(char *command, int sockfd, struct sockaddr_in addr)
{
  char linesep[] = "-----------------------------------------\n";
  // Executes the command
  FILE *outputfd = NULL;
  outputfd = popen(command, "r");
  if (outputfd == NULL)
  {
    perror("faild to run command");
    exit(1);
  }
  // Writes to the server
  char line[MAXLINE];
  char temp[MAXLINE];
  write(sockfd, linesep, strlen(linesep));
  while (fgets(line, MAXLINE, outputfd) != NULL)
  {
    strcpy(temp, line);
    write(sockfd, line, strlen(line));
  }
  char linesep2[] = "-----------------------------------------\n";
  write(sockfd, linesep2, strlen(linesep2));
}

// Get and print information about the local socket
void print_local_address(int sockfd, struct sockaddr_in addr)
{
  struct sockaddr_in server_socket;
  int server_socket_length = sizeof(addr);
  getsockname(sockfd, (sockaddr *)&server_socket, (socklen_t *)&server_socket_length);
  char buffer[INET_ADDRSTRLEN];
  const char *printable_addr = inet_ntop(AF_INET, &server_socket.sin_addr,
                                         buffer, INET_ADDRSTRLEN);
  printf("Local IP Address: %s\n", printable_addr);
  printf("Local Port: %d\n", ntohs(server_socket.sin_port));
  printf("------------------------------------\n");
}