#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXLINE 4096

int main(int argc, char **argv)
{
  int sockfd, n;
  char recvline[MAXLINE + 1];
  char error[MAXLINE + 1];
  struct sockaddr_in servaddr;

  // Input validation
  if (argc != 3)
  {
    strcpy(error, "usage: ");
    strcat(error, argv[0]);
    strcat(error, " <IPaddress>");
    strcat(error, " <#Port>");
    perror(error);
    exit(1);
  }

  // Create a socket that will use internet IPV4 and will exchange information using the
  // reliable SOCK_STREAM (TCP)
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("socket error");
    exit(1);
  }

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = ntohs((atoi(argv[2])));
  // Converts the IP address from printable format to network format
  if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
  {
    perror("inet_pton error");
    exit(1);
  }

  if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
  {
    perror("connect error");
    exit(1);
  }

  // Exercise 6
  // Get and print information about the socket
  struct sockaddr_in server_socket;
  int server_socket_length = sizeof(servaddr);
  getsockname(sockfd, (struct sockaddr *)&server_socket, (socklen_t *)&server_socket_length);
  char buffer[INET_ADDRSTRLEN];
  const char *printable_addr = inet_ntop(AF_INET, &server_socket.sin_addr,
                                         buffer, INET_ADDRSTRLEN);

  printf("IP Address: %s\n", printable_addr);

  printf("Port: %d\n", server_socket.sin_port);

  while ((n = read(sockfd, recvline, MAXLINE)) > 0)
  {
    recvline[n] = 0;
    if (fputs(recvline, stdout) == EOF)
    {
      perror("fputs error");
      exit(1);
    }
  }

  if (n < 0)
  {
    perror("read error");
    exit(1);
  }

  exit(0);
}
