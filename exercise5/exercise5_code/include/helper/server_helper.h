#ifndef SERVER_HELPER
#define SERVER_HELPER
#include "base.h"

using namespace std;

typedef struct sockaddr_in sockaddr_in;

// Validate the input
void validate_input(int argc, char **argv, char *error);

// Given a socket filedescriptor, get the IP address
string get_ip_address(int sockfd, struct sockaddr_in servaddr);

// Create output file to store each user's wins and looses
FILE *create_file(string filepath);

#endif