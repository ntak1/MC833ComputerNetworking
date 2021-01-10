#ifndef SERVER_HELPER
#define SERVER_HELPER
#include <map>
#include "base.h"

using namespace std;

typedef struct sockaddr_in sockaddr_in;

// Validate the input
void validate_input(int argc, char **argv, char *error);

// Given a socket filedescriptor, get the IP address
string get_ip_address(int sockfd, struct sockaddr_in servaddr);

// Create output file to store each user's wins and looses
FILE *create_file(string filepath);

class Player {
  public: 
    string player_name;
    int wins;
    int losses;
    string address;
    int port;

    string toString() {
      return "Player name: " + player_name + "\n"
            + "Wins: " + to_string(wins) + "\n"
            + "Losses: " + to_string(losses) + "\n"
            + "Address: " + address + "\n"
            + "Port: " + to_string(port) + "\n";
    }
};

class Punctuation {
  private:
    map<string, Player> punctuation;

  public:
    int get_wins(string player_name) {
      return punctuation[player_name].wins;
    }

    int get_losses(string player_name) {
      return punctuation[player_name].losses;
    }

    int get_total_matches(string player_name) {
      return punctuation[player_name].losses + punctuation[player_name].losses;
    }

    string get_address(string player_name) {
      return punctuation[player_name].address;
    }

    int get_port(string player_name) {
      return punctuation[player_name].port;
    }

    Player get_player(string player_name) {
      return punctuation[player_name];
    }

    Player insertPlayer(string player_name, string address, int port) {
      Player entry;
      entry.player_name = player_name;
      entry.address = address;
      entry.port = port;
      entry.losses = 0;
      entry.wins = 0;
      punctuation[player_name] = entry;
      return entry;
    }

    void addWinCount(string player_name) {
      punctuation[player_name].wins++;
    }

    void addLossCount(string player_name) {
      punctuation[player_name].losses++;
    }
};
#endif