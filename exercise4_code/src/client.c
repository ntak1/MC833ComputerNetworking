#include "syscalls/syscalls.h"
#include "helper/client_helper.h"

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
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);
    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
      perror("connect error");
      exit(1);
    }

    print_local_address(sockfd, addr);

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
        sleep(2); // Add some delay so one client do net consume all the commands and we can test several clients
        close(sockfd);
        break;
      }
    }
  }

  return 0;
}

