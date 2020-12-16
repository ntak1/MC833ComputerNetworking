#ifndef SERVER_HELPER
#define SERVER_HELPER
#include "base.h"

#define MAXCOMMANDS 100

// Given a socket filedescriptor, get the IP address
char *get_ip_address(int sockfd, struct sockaddr_in servaddr);

// Validate the input
void validate_input(int argc, char **argv, char *error);

// Get and print information about the connection socket
void write_client_address(int sockfd, struct sockaddr_in servaddr, FILE *file);

// Load the commands the server will issue to the clients from a file
char **load_commands_from_file(FILE *inputfd, int *num_commands);
#endif