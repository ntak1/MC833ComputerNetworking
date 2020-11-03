#include "syscalls.h"

#define LISTENQ 10
#define MAXDATASIZE 100
#define MAXIPLENGTH 20
#define MAXLINE 4096

int main(int argc, char **argv)
{
  int listenfd;
  int connfd;
  struct sockaddr_in servaddr;
  char buf[MAXDATASIZE];
  time_t ticks;

  char port_str[MAXIPLENGTH] = {0};
  char error[MAXLINE + 1] = {0};

  if (argc != 2)
  {
    strcpy(error, "usage: ");
    strcat(error, argv[0]);
    strcat(error, " <IPaddress>");
    strcat(error, " <#Port>");
    perror(error);
    exit(1);
  }
  strcpy(port_str, argv[1]);

  listenfd = Socket(AF_INET, SOCK_STREAM, 0);

  // Define IP address for IPV4 protocol and port number
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  servaddr.sin_port = htons(atoi(port_str));

  Bind(listenfd, (sockaddr *)&servaddr);

  // Get the port number chosen by the system and show on stdout
  struct sockaddr_in curr_addr;
  unsigned long addr_lenght = sizeof(servaddr);
  int status = getsockname(listenfd, (struct sockaddr *)&curr_addr, (socklen_t *)&addr_lenght);
  if (!status)
  {
    printf("Port: %u\n", ntohs(curr_addr.sin_port));
  }

  Listen(listenfd, LISTENQ);

  // Wait for connection requests
  for (;;)
  {
    connfd = Accept(listenfd, (struct sockaddr *)NULL, NULL);

    ticks = time(NULL);

    // Put the connection time in the buffer and write it to the connection socket
    snprintf(buf, sizeof(buf), "%.24s\r\n", ctime(&ticks));
    write(connfd, buf, strlen(buf));

    // Wait in seconds
    unsigned int sleep_sec = 3600;
    sleep(sleep_sec);

    // Close the connection
    close(connfd);
  }
  return (0);
}
