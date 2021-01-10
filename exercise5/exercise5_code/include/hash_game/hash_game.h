#ifndef HASH_GAME
#define HASH_GAME
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

      printf("   | 1 | 2 | 3 |\n");
      printf("----------------\n");
      for (int row = BOARD_START; row <= BOARD_END; row++) {
        for (int col = BOARD_START; col <= BOARD_END; col++) {
          if(col == BOARD_START) {
            printf(" %c |%2c ", 'a' + row - 1,  board[row][col]);
          } else{
            printf("|%2c ", board[row][col]);
          }
        }
        printf("|\n----------------\n");
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

#endif