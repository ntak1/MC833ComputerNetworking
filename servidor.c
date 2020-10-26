#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>

#define LISTENQ 10
#define MAXDATASIZE 100

void handle_bind_error(int error_code)
{
   switch (error_code)
   {
   case EACCES:
      printf("The requested address is protected, and the current user has inadequate permission to access it.");
      break;
   case EADDRINUSE:
      printf("The specified address is already in use");
   case EADDRNOTAVAIL:
      printf("The specified address is not available from the local machine.");
   default:
      printf("Unspecified error on bind");
      break;
   }
}

void handle_accept_error(int error_code)
{
   switch (error_code)
   {
   case EBADF:
      printf("socket is not a valid file descriptor.\n");
      break;
   case ECONNABORTED:
      printf("The connection to socket has been aborted.\n");
   case EFAULT:
      printf("The address parameter is not in a writable part of the user address space.\n");
   case EINTR:
      printf("The accept() system call was terminated by a signal.\n");
   case EINVAL:
      printf("socket is unwilling to accept connections.\n");
   default:
      printf("Error accepting the connection.\n");
      break;
   }
}

int main(int argc, char **argv)
{
   int listenfd;
   int connfd;
   struct sockaddr_in servaddr;
   char buf[MAXDATASIZE];
   time_t ticks;

   // Create a socket IPV4 using TCP protocol
   if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
   {
      perror("socket");
      exit(1);
   }

   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
   servaddr.sin_port = 0;

   if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
   {
      perror("bind");
      handle_bind_error(errno);
      exit(1);
   }

   struct sockaddr_in curr_addr;
   unsigned long addr_lenght = sizeof(servaddr);
   int status = getsockname(listenfd, (struct sockaddr *)&curr_addr, (socklen_t *)&addr_lenght);
   if (!status)
   {
      printf("Port: %d\n", curr_addr.sin_port);
   }

   if (listen(listenfd, LISTENQ) == -1)
   {
      perror("listen");
      exit(1);
   }

   for (;;)
   {
      if ((connfd = accept(listenfd, (struct sockaddr *)NULL, NULL)) == -1)
      {
         handle_accept_error(errno);
         perror("accept");
         exit(1);
      }

      
      ticks = time(NULL);

      snprintf(buf, sizeof(buf), "%.24s\r\n", ctime(&ticks));
      write(connfd, buf, strlen(buf));

      close(connfd);
   }
   return (0);
}
