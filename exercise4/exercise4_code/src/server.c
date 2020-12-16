/* Author: Naomi Takemoto
 * RA: 184849
 * December 2020
 */
#include <unistd.h>
#include "syscalls/syscalls.h"
#include "helper/server_helper.h"

#define LISTENQ 10
#define MAXDATASIZE 100
#define MAXIPLENGTH 20
#define MAXLINE 4096
#define MAXCOMMANDS 100
#define LOCALHOST "127.0.0.1"

/* This server echoes the content sent by the client */
int main(int argc, char **argv)
{

  //--------------------------------------------------------------------
  // Set this variable any value other than 0 to be in "verbose" mode
  char verbose = 1;
  //--------------------------------------------------------------------

  // Connection variables
  int listenfd;
  int connfd;
  struct sockaddr_in servaddr;
  struct sockaddr_in clientaddr;
  char buf[MAXDATASIZE];
  char port_str[MAXIPLENGTH] = {0};
  char error[MAXLINE + 1] = {0};
  time_t ticks;

  char buffer[MAXLINE + 1] = {0};
  int n = 0;

  // Output file to store the connection and disconenctions
  FILE *connections_fd = NULL;
  connections_fd = fopen("connections.txt", "w");
  if (connections_fd == NULL)
  {
    perror("Error opening file");
    exit(1);
  }

  // Input: args
  validate_input(argc, argv, error);
  strcpy(port_str, argv[1]);

  listenfd = Socket(AF_INET, SOCK_STREAM, 0);

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  // servaddr.sin_addr.s_addr = inet_addr(LOCALHOST);
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(atoi(port_str));

  Bind(listenfd, (sockaddr *)&servaddr);

  // Get the port number chosen by the system and show on stdout
  struct sockaddr_in curr_addr;
  unsigned long addr_lenght = sizeof(servaddr);
  int status = getsockname(listenfd, (sockaddr *)&curr_addr, (socklen_t *)&addr_lenght);
  if (!status && verbose)
  {
    printf("Server listening on port: %u\n", ntohs(curr_addr.sin_port));
  }
  else if (status != 0)
  {
    perror("Error defining server port.");
    exit(0);
  }

  if (verbose != 0)
  {
    printf("Waiting connection ...\n");
    fflush(stdout);
  }

  Listen(listenfd, LISTENQ);

  printf("See client connections and disconnection information in connections.txt\n");
  printf("[parent] Parent pid %u\n", getpid());

  while (TRUE)
  {

    // Connects to the client
    socklen_t len_client_addr = sizeof(clientaddr);
    connfd = Accept(listenfd, (sockaddr *)&clientaddr, &len_client_addr);
    snprintf(buf, sizeof(buf), "Client with %s port %d CONNECTED at: %.24s\r\n",
             get_ip_address(connfd, clientaddr),
             ntohs(clientaddr.sin_port),
             ctime(&ticks));
    fputs(buf, connections_fd);
    printf("%s", buf);

    // Reads the result sent by the client and writes in the output file
    while ((n = read(connfd, buffer, MAXLINE)) > 0)
    {
      buffer[n] = 0;
      int success_bytes = write(connfd, buffer, n);
      if (success_bytes <= 0)
      {
        close(connfd);
        exit(1);
      }
    }
    // If the client has closed the connection
    if (n == 0)
    {
      close(connfd);
      exit(0);
    }
  }

  return (0);
}