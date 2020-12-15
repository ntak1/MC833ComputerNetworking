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

int main(int argc, char **argv)
{
   int listenfd;
   int connfd;
   struct sockaddr_in addr;
   char buf[MAXDATASIZE];
   time_t ticks;

   // Create a socket IPV4 using TCP protocol
   if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
   {
      perror("socket");
      exit(1);
   }

   // Define IP address for IPV4 protocol and port number
   bzero(&addr, sizeof(addr));
   addr.sin_family = AF_INET;
   addr.sin_addr.s_addr = inet_addr("127.0.0.1");
   addr.sin_port = htons(0); // System chooses the port number

   // Associate the socket with the address defined above
   if (bind(listenfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
   {
      perror("bind");
      exit(1);
   }

   // Get the port number chosen by the system and show on stdout
   struct sockaddr_in curr_addr;
   unsigned long addr_lenght = sizeof(addr);
   int status = getsockname(listenfd, (struct sockaddr *)&curr_addr, (socklen_t *)&addr_lenght);
   if (!status)
   {
      printf("Port: %u\n", ntohs(curr_addr.sin_port));
   }

   // Indicates that the server is ready to accept incoming connections
   // Define the queue limit for the number of connections to the socket (LISTEQ)
   if (listen(listenfd, BACKLOG) == -1)
   {
      perror("listen");
      exit(1);
   }

   // Wait for connection requests
   for (;;)
   {
      // Accept a connection, generating a descriptor for the connection socket
      // connfd
      if ((connfd = accept(listenfd, (struct sockaddr *)NULL, NULL)) == -1)
      {
         perror("accept");
         exit(1);
      }
      else
      {
         // Get information about the socket related to connfc
         struct sockaddr_in client_socket_addr;
         socklen_t addr_len = INET_ADDRSTRLEN;
         getpeername(connfd, (struct sockaddr *)&client_socket_addr, (socklen_t *)&addr_len);
         char buffer[INET_ADDRSTRLEN];

         // Get the printable form of the address
         const char *printable_addr = inet_ntop(client_socket_addr.sin_family, &client_socket_addr.sin_addr,
                                                buffer, addr_len);

         printf("----------------------------\n");
         printf("IP Address: %s\n", printable_addr);
         printf("Address len: %d\n", addr_len);
         printf("Port: %d\n", ntohs(client_socket_addr.sin_port));
         printf("----------------------------\n");
      }

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
