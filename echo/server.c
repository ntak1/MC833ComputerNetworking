/* Author: Naomi Takemoto
 * RA: 184849
 * November 2020
 * 
 * For report 3, exercise 3: change the value in the BACKLOG macro
 */
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include "syscalls.h"

// Change this value
#define BACKLOG 5

#define MAXDATASIZE 100
#define MAXIPLENGTH 20
#define MAXLINE 4096
#define MAXCOMMANDS 100
#define LOCALHOST "127.0.0.1"
#define NUM_REPEATS 5

// Pragmas
void validate_input(int argc, char **argv, char *error);
void write_client_address(int sockfd, struct sockaddr_in servaddr, FILE *file);
char **load_commands_from_file(FILE *inputfd, int *num_commands);
char *get_ip_address(int sockfd, struct sockaddr_in servaddr);


// Signal handler to avoid Zombies
void sig_chld(int signo) 
{
  pid_t pid;
  int stat;
  pid = wait(&stat);
  printf("child %d terminated\n", pid);
  return;
}

/* Send commands to the client and waits for the result.
 * Write the result to a file. */
int main(int argc, char **argv)
{

  //--------------------------------------------------------------------
  // Set this variable any value other than 0 to be in "verbose" mode
  char verbose = 1;
  //--------------------------------------------------------------------

  // Connection variables
  int listenfd;
  int connfd;
  struct sockaddr_in servaddr;
  struct sockaddr_in clientaddr;
  char buf[MAXDATASIZE];
  char port_str[MAXIPLENGTH] = {0};
  char error[MAXLINE + 1] = {0};
  time_t ticks = 0L;

  // Keep track of the current command
  char **commands;

  // Auxiliary variables
  int num_commands = 0;

  // Output file (stores the command result)
  FILE *command_output_fd = NULL;
  command_output_fd = fopen("server_output.txt", "w");
  if (command_output_fd == NULL)
  {
    perror("Error opening file");
    exit(1);
  }

  // Output file to store the connection and disconenctions
  FILE *connections_fd = NULL;
  connections_fd = fopen("connections.txt", "w");
  if (connections_fd == NULL)
  {
    perror("Error opening file");
    exit(1);
  }

  // Commands file
  FILE *inputfd = NULL;
  inputfd = fopen("arq01.in", "r");
  if (inputfd == NULL)
  {
    perror("Error opening file");
    exit(1);
  }

  // Input: args
  validate_input(argc, argv, error);
  strcpy(port_str, argv[1]);

  // Command list
  commands = load_commands_from_file(inputfd, &num_commands);

  listenfd = Socket(AF_INET, SOCK_STREAM, 0);

  signal (SIGCHLD, sig_chld);

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr(LOCALHOST);
  servaddr.sin_port = htons(atoi(port_str));

  Bind(listenfd, (sockaddr *)&servaddr);

  // Get the port number chosen by the system and show on stdout
  struct sockaddr_in curr_addr;
  unsigned long addr_lenght = sizeof(servaddr);
  int status = getsockname(listenfd, (sockaddr *)&curr_addr, (socklen_t *)&addr_lenght);
  if (!status && verbose)
  {
    printf("Server listening on port: %u\n", ntohs(curr_addr.sin_port));
  }
  else if (status != 0)
  {
    perror("Error defining server port.");
    exit(0);
  }

  if (verbose != 0)
  {
    printf("Waiting connection ...\n");
    fflush(stdout);
  }

  Listen(listenfd, BACKLOG);

  printf("See client connections and disconnection information in connections.txt\n");
  printf("See commands outputs in server_ouput.txt\n");
  printf("[parent] Parent pid %u\n", getpid());

  // Wait for connection requests
  int conn_number = -1;
  while (NUM_REPEATS)
  {
    socklen_t len_client_addr = sizeof(clientaddr);
    connfd = Accept(listenfd, (sockaddr *)&clientaddr, &len_client_addr);

    // For each connection fork
    conn_number++;
    conn_number %= num_commands;
    int pid = 0;
    if ((pid = fork()) == 0)
    {
      close(listenfd);
      if (verbose != 0)
      {
        printf("Connected!\n");
        write_client_address(connfd, clientaddr, command_output_fd);
      }
      printf("[child] Parent pid %u\n", getppid());
      // Write the start connection instant
      ticks = time(NULL);
      snprintf(buf, sizeof(buf), "Client with %s port %u CONNECTED at: %.24s\r\n",
               get_ip_address(connfd, clientaddr),
               ntohs(clientaddr.sin_port),
               ctime(&ticks));
      fputs(buf, connections_fd);

      // Sends the command to the client
      snprintf(buf, sizeof(buf), "%s", commands[conn_number]);
      write(connfd, buf, strlen(buf));

      // Reads the result sent by the client and writes in the output file
      int n = 0;
      char *exit_buffer = (char *)malloc(sizeof(char) * MAXLINE);
      while ((n = read(connfd, exit_buffer, MAXLINE)) > 0)
      {
        exit_buffer[n] = 0;
        if (fputs(exit_buffer, command_output_fd) == EOF)
        {
          perror("fputs error");
          exit(1);
        }
        if (verbose)
        {
          printf("%s", exit_buffer);
        }
      }

      // Connection close instant
      sleep(1);
      ticks = time(NULL);
      snprintf(buf, sizeof(buf), "Client with %s port %u DISCONNECTED at: %.24s\r\n",
               get_ip_address(connfd, clientaddr),
               ntohs(clientaddr.sin_port),
               ctime(&ticks));
      fputs(buf, connections_fd);

      // If the client has closed the connection after receiving an "exit"
      if (n == 0)
      {
        close(connfd);
        exit(0);
      }
    }

    close(connfd);
  }

  fclose(command_output_fd);
  return (0);
}

// ----------------------------------------------------------------------------
// Helper functions implementation
// ----------------------------------------------------------------------------

// Given a socket filedescriptor, get the IP address
char *get_ip_address(int sockfd, struct sockaddr_in servaddr)
{
  struct sockaddr_in server_socket;
  int server_socket_length = sizeof(servaddr);
  getsockname(sockfd, (sockaddr *)&server_socket, (socklen_t *)&server_socket_length);
  char buffer[INET_ADDRSTRLEN];
  const char *printable_addr = inet_ntop(AF_INET, &server_socket.sin_addr,
                                         buffer, INET_ADDRSTRLEN);
  char *ans = malloc(strlen(printable_addr) * sizeof(char));
  strcpy(ans, printable_addr);
  return ans;
}

// Validate the input
void validate_input(int argc, char **argv, char *error)
{
  if (argc != 2)
  {
    strcpy(error, "usage: ");
    strcat(error, argv[0]);
    strcat(error, " <#Port>");
    perror(error);
    exit(1);
  }
}

// Get and print information about the connection socket
void write_client_address(int sockfd, struct sockaddr_in servaddr, FILE *file)
{
  char *printable_addr = get_ip_address(sockfd, servaddr);
  char message[MAXLINE];
  strcpy(message, "Client IP Address: ");
  strcat(message, printable_addr);
  strcat(message, "\n");
  fputs(message, file);

  char port_str[50];
  strcpy(message, "Client Port: ");
  sprintf(port_str, "%u", ntohs(servaddr.sin_port));
  strcat(message, port_str);
  strcat(message, "\n");
  fputs(message, file);
}

// Load the commands the server will issue to the clients from a file
char **load_commands_from_file(FILE *inputfd, int *num_commands)
{
  char command_line[MAXLINE];
  char **commands = (char **)malloc(sizeof(char *) * MAXCOMMANDS);
  if (commands == NULL)
  {
    perror("Error allocating memory.");
    exit(1);
  }
  int i = 0;
  while (fgets(command_line, MAXLINE, inputfd) != NULL)
  {
    commands[i] = (char *)malloc(sizeof(char) * MAXLINE);
    strcpy(commands[i], command_line);
    i++;
  }
  (*num_commands) = i;
  if (i == 0)
  {
    perror("No commands specified.");
    exit(1);
  }
  return commands;
}