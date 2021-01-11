#include "syscalls.h"

// Create a socket IPV4 using TCP protocol
int Socket(int family, int socket_type, int protocol)
{
  int listenfd = -1;
  if ((listenfd = socket(family, socket_type, protocol)) == -1)
  {
    perror("socket");
    exit(1);
  }
  return listenfd;
}

// Associate the socket with the address defined above
void Bind(int fd, sockaddr *socket_addr)
{
  if (bind(fd, socket_addr, sizeof(*socket_addr)) == -1)
  {
    perror("bind");
    exit(1);
  }
}

// Indicates that the server is ready to accept incoming connections
// Define the queue limit for the number of connections to the socket (LISTEQ)
void Listen(int fd, int queue_size)
{
  if (listen(fd, queue_size) == -1)
  {
    perror("listen");
    exit(1);
  }
}

// The listening socket accepts the conection and returns a socket descriptor for the connection
int Accept(int listenfd, struct sockaddr *__restrict__ address)
{
  int connfd = -1;
  socklen_t len_socket_addr = sizeof((*address));
  if ((connfd = accept(listenfd, (struct sockaddr *)address, &len_socket_addr)) == -1)
  {
    perror("accept");
    exit(1);
  }
  return connfd;
}

 // Used by the client to establish a connection with the server
 void Connect(int socket_fd, const struct sockaddr *server_addr) {
    if (connect(socket_fd, server_addr, sizeof(server_addr)) < 0)
    {
      perror("connect error");
      exit(1);
    }
 }