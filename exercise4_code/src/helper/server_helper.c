#include "server_helper.h"

// Given a socket filedescriptor, get the IP address
char *get_ip_address(int sockfd, struct sockaddr_in servaddr)
{
  struct sockaddr_in server_socket;
  int server_socket_length = sizeof(servaddr);
  getsockname(sockfd, (sockaddr *)&server_socket, (socklen_t *)&server_socket_length);
  char buffer[INET_ADDRSTRLEN];
  const char *printable_addr = inet_ntop(AF_INET, &server_socket.sin_addr,
                                         buffer, INET_ADDRSTRLEN);
  char *ans = malloc(strlen(printable_addr) * sizeof(char));
  strcpy(ans, printable_addr);
  return ans;
}

// Validate the input
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

// Get and print information about the connection socket
void write_client_address(int sockfd, struct sockaddr_in servaddr, FILE *file)
{
  char *printable_addr = get_ip_address(sockfd, servaddr);
  char message[MAXLINE];
  strcpy(message, "Client IP Address: ");
  strcat(message, printable_addr);
  strcat(message, "\n");
  fputs(message, file);

  char port_str[50];
  strcpy(message, "Client Port: ");
  sprintf(port_str, "%u", ntohs(servaddr.sin_port));
  strcat(message, port_str);
  strcat(message, "\n");
  fputs(message, file);
}

// Load the commands the server will issue to the clients from a file
char **load_commands_from_file(FILE *inputfd, int *num_commands)
{
  char command_line[MAXLINE];
  char **commands = (char **)malloc(sizeof(char *) * MAXCOMMANDS);
  if (commands == NULL)
  {
    perror("Error allocating memory.");
    exit(1);
  }
  int i = 0;
  while (fgets(command_line, MAXLINE, inputfd) != NULL)
  {
    commands[i] = (char *)malloc(sizeof(char) * MAXLINE);
    strcpy(commands[i], command_line);
    i++;
  }
  (*num_commands) = i;
  if (i == 0)
  {
    perror("No commands specified.");
    exit(1);
  }
  return commands;
}