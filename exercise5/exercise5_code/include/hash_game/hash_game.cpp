#include <iostream>
#include <vector>
#include <string>
#include <cstdio>

using namespace std;

class Player {
  public:
    string id;
    char symbol;
    Player(string id, char symbol) {
      this->id = id;
      this->symbol = symbol;
    }
};

class Game {
  private:
    int board[4][4];
    int turn;
    Player *player1;
    Player *player2;
    Player *winner;
    Player *current_player;
    int BOARD_START;
    int BOARD_END;

    inline bool position_empty(int row, int col) {
      return board[row][col] == ' ';
    }

    inline bool position_empty(pair<int, int> position) {
      return board[position.first][position.second] == ' ';
    }

  public:
    Game(Player *player1, Player *player2) {
      this->player1 = player1;
      this->player2 = player2;
      this->turn = 0;
      this->BOARD_START = 1;
      this->BOARD_END = 3;
      this->current_player = player1;
      this->winner = NULL;
      fill(*board, *board + 16, ' ');
    }

    void print_board() {
      printf("\nMove %d\n", this->turn);
      printf("Next Player [%c]: %s\n", current_player->symbol, current_player->id.c_str());
      printf("-------------\n");
      for (int row = BOARD_START; row <= BOARD_END; row++) {
        for (int col = BOARD_START; col <= BOARD_END; col++) {
          printf("|%2c ", board[row][col]);
        }
        printf("|\n-------------\n");
      }

      if(hash()) {
        printf("GAME OVER! WINNER %s\n", winner->id.c_str());
        return;
      }
      if(board_full()) {
        printf("GAME OVER! DRAW\n");
      }
    }

    bool board_full() {
      for (int col = BOARD_START; col <= BOARD_END; col++) {
        for (int row = BOARD_START; row <= BOARD_END; row++) {
          if (position_empty(row, col)) {
            return false;
          }
        }
      }
      printf("Board is full!\n");
      return true;
    }

    bool hash() {
      // Columns
      bool hash = true;
      int row = 0;
      int col = 0;

      for (col = BOARD_START; col <= BOARD_END; col++) {
        for(row = BOARD_START; row < BOARD_END; row++){
          hash = hash && (board[row][col] == board[row+1][col]); 
        }
        if (hash && !position_empty(row, col)) {
          // printf("Column hash!\n");
          return true;
        }
        hash = true;
      }

      // Rows
      for(row = BOARD_START; row < BOARD_END; row++){
        for (col = BOARD_START; col <= BOARD_END; col++) {
          hash = hash && (board[row][col] == board[row][col+1]); 
        }
        if (hash && !position_empty(row, col)) {
          // printf("Row hash!\n");
          return true;
        }
        hash = true;
      }

      // Diagonal
      bool diagonal1 = true;
      bool diagonal2 = true;
      for(int i = BOARD_START; i < BOARD_END; i++) {
        diagonal1 = diagonal1 && (board[i][i] == board[i+1][i+1]) && !position_empty(i, i);
        diagonal2 = diagonal2 && (board[BOARD_END - i + 1][i]  == board[BOARD_END - i][i +1]) 
                                    && !position_empty(BOARD_END - i + 1, i);
      }
    //   if (diagonal1) {
    //     // printf("Main diagonal hash!\n");
    //   }
    //   if (diagonal2) {
    //     printf("Non main diagonal hash!\n");
    //   }
      return diagonal1 || diagonal2;
    }

    bool game_over() {
      return hash() || board_full();
    }

    int get_turn() {
      return turn;
    }

    bool invalid_position(pair<int, int> position) {
      int row = position.first;
      int col = position.second;
      if (row < BOARD_START || row > BOARD_END || col < BOARD_START || col > BOARD_END) {
        printf("Error: position out of bounds!");
        return true;
      }
      if (!position_empty(position)) {
        printf("Error: position is not empty!");
        return true;
      }
      return false;
    }

    bool make_move(Player *player, pair<int,int> position) {
     turn++;
     if (invalid_position(position)) {
       return false;
     }
     if (player->id.compare(player1->id) == 0) {
       board[position.first][position.second] = player1->symbol;
       current_player = player2;
     } else {
       board[position.first][position.second] = player2->symbol;
       current_player = player1;
     }
     if(hash()) {
       winner = player;
       return true;
     }
     if(board_full()) {
       printf("GAME OVER! DRAW!\n");
     }
     return true;
    }
};

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