#include "../include/helper/client_helper.h"
#include "../include/hash_game/hash_game.h"

using namespace std;

int main(void) {
  string player1_id = "player 1";
  string player2_id = "player 2";

  Player *player1 = new Player(player1_id, 'X');
  Player *player2 = new Player(player2_id, 'O');

  vector<pair<int, int> > player1_moves;
  player1_moves.push_back(pair<int,int>(1,1));
  player1_moves.push_back(pair<int,int>(2,2));
  player1_moves.push_back(pair<int,int>(3,2));
  player1_moves.push_back(pair<int,int>(3,1));
  player1_moves.push_back(pair<int,int>(3,3));


  vector<pair<int, int> > player2_moves;
  player2_moves.push_back(pair<int,int>(1,2));
  player2_moves.push_back(pair<int,int>(1,3));
  player2_moves.push_back(pair<int,int>(2,1));
  player2_moves.push_back(pair<int,int>(2,3));

  Game *game = new Game(player1, player2);

  int i = 0;
  int j = 0;
  game->print_board();
  while(!(game->game_over())) {
    game->make_move(player1, player1_moves[i++]);
    game->print_board();
    if (game->game_over()) {
      break;
    }
    game->make_move(player2, player2_moves[j++]);
    game->print_board();
  }
}