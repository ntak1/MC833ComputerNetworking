#ifndef SERVER_HELPER
#define SERVER_HELPER
#include <map>
#include <string>
#include <vector>
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
    int draws;
    string address;
    int port;
    bool available;

    string toString() {
      return "Player name: " + player_name + "\n"
            + "Wins: " + to_string(wins) + "\n"
            + "Losses: " + to_string(losses) + "\n"
            + "Draws: " + to_string(draws) + "\n"
            + "Address: " + address + "\n"
            + "Available: " + to_string(available) + "\n";
            + "Port: " + to_string(port) + "\n";
    }
};

class PlayersContainer {
  private:
    map<string, Player> players;

  public:
    Player getPlayer(string player_name) {
      return players[player_name];
    }

    Player insertOrUpdatePlayer(string player_name, string address, int port) {
      auto it = players.find(player_name);
      if (it == players.end()) {
        Player entry;
        entry.player_name = player_name;
        entry.address = address;
        entry.port = port;
        entry.losses = 0;
        entry.wins = 0;
        entry.available = false;
        entry.draws = 0;
        players[player_name] = entry;
      } else {
        players[player_name].address = address;
        players[player_name].port = port;
      }
    }

    void addWinCount(string player_name) {
      players[player_name].wins++;
    }

    void addLossCount(string player_name) {
      players[player_name].losses++;
    }

    void addDrawCount(string player_name) {
      players[player_name].draws++;
    }

    vector<string> getAvailablePlayersIds() {
      vector<string> ans;
      for(auto const& player: players) {
        if(player.second.available) {
          ans.push_back(player.first);
        }
      }
      return ans;
    }

   map<string, Player> getPlayers() {
     return players;
   }

};
#endif