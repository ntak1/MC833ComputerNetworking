#include "syscalls/syscalls.h"
#include "helper/client_helper.h"

int max(int a, int b)
{
  return a < b ? a : b;
}

/* This client performs the following actions
 *   Connect to the server
 *   Read the input from stdin
 *   Send each line read to the server
 *   Waits for the server to echo the result
 *   After all the text is echoed, stop
 */
int main(int argc, char **argv)
{
  int sockfd, n;
  char recvline[MAXLINE + 1];
  char error[MAXLINE + 1];
  struct sockaddr_in addr;

  int maxfdp1, stdineof;
  fd_set rset;

  validate_input(argc, argv, error);

  // Set family, ip and port address
  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons((atoi(argv[2])));
  if (inet_pton(AF_INET, argv[1], &addr.sin_addr) <= 0)
  {
    perror("inet_pton error");
    exit(1);
  }

  // Prepare variables to use select
  stdineof = FALSE;
  FD_ZERO(&rset);

  // Connect to the server
  sockfd = Socket(AF_INET, SOCK_STREAM, 0);
  if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    perror("connect error");
    exit(1);
  };

  // Main loop
  while (TRUE)
  {
    // Select standard input to redability
    if (stdineof == FALSE)
    {
      FD_SET(fileno(stdin), &rset);
    }
    FD_SET(sockfd, &rset);
    maxfdp1 = max(fileno(stdin), sockfd) + 1;
    select(maxfdp1, &rset, NULL, NULL, NULL);

    // If socket is readable
    if (FD_ISSET(sockfd, &rset))
    {
      // Nothing is read
      if ((n = read(sockfd, recvline, MAXLINE)) == 0)
      {
        // Normal termination condition
        if (stdineof == TRUE)
        {
          exit(0);
        }
        else
        {
          perror("Server terminated prematurely");
          exit(1);
        }
      }
      // Write the output
      recvline[n] = 0;
      if (fputs(recvline, stdout) == EOF)
      {
        perror("fputs error");
        exit(1);
      }
    }

    // If input is readable
    if (FD_ISSET(fileno(stdin), &rset))
    {
      // And nothing is read
      // Once the client finishes reading the stdin, close the write half of the connection
      // with the server
      if ((n = read(fileno(stdin), recvline, MAXLINE)) == 0)
      {
        stdineof = TRUE;
        shutdown(sockfd, SHUT_WR);
        FD_CLR(fileno(stdin), &rset);
        continue;
      }
      // Write to the server
      recvline[n] = 0;
      printf("%s", recvline);
      fflush(stdout);
      int success_bytes = write(sockfd, recvline, n);
      if (success_bytes <= 0)
      {
        perror("Unable to communicate with the server.");
        exit(1);
      }
    }
  }

  return 0;
}
