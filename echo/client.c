#include "syscalls.h"
#define MAXLINE 4096

// Pragmas
void validate_input(int argc, char **argv, char error[]);
void print_local_address(int sockfd, struct sockaddr_in addr);
void send_command_result(char *command, int sockfd, struct sockaddr_in addr);

/* Connect to the server and listen to it's request
 * Execute command
 * Send the result back to the server */
int main(int argc, char **argv)
{
  int sockfd, n;
  char recvline[MAXLINE + 1];
  char error[MAXLINE + 1];
  struct sockaddr_in addr;

  // Input validation
  validate_input(argc, argv, error);

  // Set family, ip and port address
  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = ntohs((atoi(argv[2])));

  // Converts the IP address from host format to network format
  if (inet_pton(AF_INET, argv[1], &addr.sin_addr) <= 0)
  {
    perror("inet_pton error");
    exit(1);
  }

  for (;;)
  {
    sleep(1); // Add some delay so one client do net consume all the commands and we can test several clients
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);
    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
      perror("connect error");
      exit(1);
    }
    print_local_address(sockfd, addr);
    sleep(1); 

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
      if (!strcmp(recvline, "exit\n"))
      {
        write(sockfd, NULL, 0);
        printf("tixe\n");
        close(sockfd);
        exit(0);
      }

      // Send the command result back to the server and close the connection
      if (n > 0 && recvline[n - 1] == '\n')
      {
        send_command_result(recvline, sockfd, addr);
        close(sockfd);
        exit(0);
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