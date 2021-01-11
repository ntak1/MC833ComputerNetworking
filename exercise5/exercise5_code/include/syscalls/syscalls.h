#ifndef SYSCALLS

#define SYSCALLS
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

int Socket(int socket_family, int socket_type, int protocol);

void Bind(int, struct sockaddr *);

void Listen(int, int);

int Accept(int, struct sockaddr *__restrict__);

void Connect(int socket_fd, const struct sockaddr *);

#endif