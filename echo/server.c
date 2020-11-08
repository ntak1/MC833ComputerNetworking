#include <unistd.h>
#include "syscalls.h"

#define LISTENQ 10
#define MAXDATASIZE 100
#define MAXIPLENGTH 20
#define MAXLINE 4096
#define MAXCOMMANDS 100
#define LOCALHOST "127.0.0.1"

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

/* Send commands to the client and waits for the result.
 * Write the result to a file.
 */

int main(int argc, char **argv)
{
  // Connection variables
  int listenfd;
  int connfd;
  struct sockaddr_in servaddr;
  char buf[MAXDATASIZE];
  char port_str[MAXIPLENGTH] = {0};
  char error[MAXLINE + 1] = {0};

  // Keep track of the current command
  char **commands;
  char command_line[MAXLINE];

  // Auxiliary variables
  int i = 0;

  // Output file (stores the command result)
  FILE *command_output_fd = NULL;
  command_output_fd = fopen("server_output.txt", "w");
  if (command_output_fd == NULL)
  {
    perror("Error opening file");
    exit(1);
  }

  // Input: args
  validate_input(argc, argv, error);
  strcpy(port_str, argv[1]);

  // Command list
  commands = (char **)malloc(sizeof(char *) * MAXCOMMANDS);
  if (commands == NULL)
  {
    perror("Error allocating memory.");
    exit(1);
  }
  i = 0;
  while (fgets(command_line, MAXLINE, stdin) != NULL)
  {
    strcmp(commands[i], command_line);
    i++;
  }
  if (i == 0)
  {
    perror("No commands specified.");
    exit(1);
  }

  listenfd = Socket(AF_INET, SOCK_STREAM, 0);

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr(LOCALHOST);
  servaddr.sin_port = htons(atoi(port_str));

  Bind(listenfd, (sockaddr *)&servaddr);

  // Get the port number chosen by the system and show on stdout
  struct sockaddr_in curr_addr;
  unsigned long addr_lenght = sizeof(servaddr);
  int status = getsockname(listenfd, (struct sockaddr *)&curr_addr, (socklen_t *)&addr_lenght);
  if (!status)
  {
    printf("Port: %u\n", ntohs(curr_addr.sin_port));
  }

  Listen(listenfd, LISTENQ);

  // Wait for connection requests
  int conn_number = -1;
  for (;;)
  {
    connfd = Accept(listenfd, (struct sockaddr *)NULL, NULL);

    // For each connection fork
    conn_number++;
    int pid = 0;
    if ((pid = fork()) == 0)
    {
      close(listenfd);
      printf("Connected\n");

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
      }
      fclose(command_output_fd);
      close(connfd);
      exit(0);
    }
    fclose(command_output_fd);
    close(connfd);
  }

  return (0);
}
