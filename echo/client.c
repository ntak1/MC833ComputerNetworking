#include "syscalls.h"
#define MAXLINE 4096

// Pragmas
void validate_input(int argc, char **argv, char error[]);
void send_command_result(char *command, int sockfd, struct sockaddr_in servaddr);
void print_local_address(int sockfd, struct sockaddr_in servaddr);

/* Connect to the server and listen to it's request
 * Execute command
 * Send the result back to the server */
int main(int argc, char **argv)
{
  int sockfd, n;
  char recvline[MAXLINE + 1];
  char error[MAXLINE + 1];
  struct sockaddr_in servaddr;

  // Input validation
  validate_input(argc, argv, error);

  // // Create an internet IPV4 socket
  // sockfd = Socket(AF_INET, SOCK_STREAM, 0);

  // Set family, ip and port address
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = ntohs((atoi(argv[2])));

  // Converts the IP address from host format to network format
  if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
  {
    perror("inet_pton error");
    exit(1);
  }

  for (;;)
  {
    // TODO: use wrapper function
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
      perror("connect error");
      exit(1);
    }

    print_local_address(sockfd, servaddr);

    // Wait for the command
    while ((n = read(sockfd, recvline, MAXLINE)) > 0)
    {
      recvline[n] = 0;
      // TODO: delete the print
      fputs("[command] ", stdout);
      if (fputs(recvline, stdout) == EOF)
      {
        perror("fputs error");
        exit(1);
      }

      // Send the command result back to the server and close the connection
      if (n > 0 && recvline[n - 1] == '\n')
      {
        send_command_result(recvline, sockfd, servaddr);
        sleep(2); // Add some delay so one client do net consume all the commands
        close(sockfd);
        break;
      }
    }
  }

  return 0;
}

// ----------------------------------------------------------------------------
// Helper functions implementation
// ----------------------------------------------------------------------------

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

// Get and print information about the local socket
void write_client_address_to_server(int sockfd, struct sockaddr_in servaddr)
{
  struct sockaddr_in server_socket;
  int server_socket_length = sizeof(servaddr);
  getsockname(sockfd, (sockaddr *)&server_socket, (socklen_t *)&server_socket_length);
  char buffer[INET_ADDRSTRLEN];
  const char *printable_addr = inet_ntop(AF_INET, &server_socket.sin_addr,
                                         buffer, INET_ADDRSTRLEN);
  char message[MAXLINE];
  strcpy(message, "Client IP Address: ");
  strcat(message, printable_addr);
  strcat(message, "\n");
  write(sockfd, message, strlen(message));

  char port_str[50];
  strcpy(message, "Client Port: ");
  sprintf(port_str, "%u", ntohs(servaddr.sin_port));
  strcat(message, port_str);
  strcat(message, port_str);

  write(sockfd, message, strlen(message));
}

// Send the command result back to the server
void send_command_result(char *command, int sockfd, struct sockaddr_in servaddr)
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
  write_client_address_to_server(sockfd, servaddr);
  write(sockfd, linesep, strlen(linesep));
  while (fgets(line, MAXLINE, outputfd) != NULL)
  {
    strcpy(temp, line);
    if (!strcmp(temp, "exit"))
    {
      write(sockfd, NULL, 0);
      exit(0);
    }
    write(sockfd, line, strlen(line));
  }
  char linesep2[] = "\n-----------------------------------------\n\n\n\n";
  write(sockfd, linesep2, strlen(linesep2));
}

// Get and print information about the local socket
void print_local_address(int sockfd, struct sockaddr_in servaddr)
{
  struct sockaddr_in server_socket;
  int server_socket_length = sizeof(servaddr);
  getsockname(sockfd, (sockaddr *)&server_socket, (socklen_t *)&server_socket_length);
  char buffer[INET_ADDRSTRLEN];
  const char *printable_addr = inet_ntop(AF_INET, &server_socket.sin_addr,
                                         buffer, INET_ADDRSTRLEN);
  printf("Local IP Address: %s\n", printable_addr);
  printf("Local Port: %d\n", server_socket.sin_port);
  printf("------------------------------------\n");
}