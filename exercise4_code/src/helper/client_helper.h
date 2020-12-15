// ----------------------------------------------------------------------------
// Helper functions implementation
// ----------------------------------------------------------------------------
#ifndef CLIENT_HELPER
#define CLIENT_HELPER
#include "base.h"

// Validate input
void validate_input(int argc, char **argv, char error[]);

// Send the command result back to the server
void send_command_result(char *command, int sockfd, struct sockaddr_in addr);

// Get and print information about the local socket
void print_local_address(int sockfd, struct sockaddr_in addr);
#endif