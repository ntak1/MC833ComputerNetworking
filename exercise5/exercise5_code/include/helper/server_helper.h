#ifndef SERVER_HELPER
#define SERVER_HELPER
#include "base.h"

#define MAXCOMMANDS 100

using namespace std;

// Given a socket filedescriptor, get the IP address
string get_ip_address(int sockfd, struct sockaddr_in servaddr);

#endif