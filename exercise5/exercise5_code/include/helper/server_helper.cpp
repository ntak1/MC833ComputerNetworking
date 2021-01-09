#include <string>
#include "server_helper.h"

using namespace std;

// Given a socket filedescriptor, get the IP address
string get_ip_address(int sockfd, struct sockaddr_in servaddr)
{
  struct sockaddr_in server_socket;
  int server_socket_length = sizeof(servaddr);
  getsockname(sockfd, (sockaddr *)&server_socket, (socklen_t *)&server_socket_length);
  char buffer[INET_ADDRSTRLEN];
  const char *printable_addr = inet_ntop(AF_INET, &server_socket.sin_addr,
                                         buffer, INET_ADDRSTRLEN);
  return string(printable_addr);
}