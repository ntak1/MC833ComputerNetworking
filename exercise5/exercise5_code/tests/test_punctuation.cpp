#include <cstdio>
#include "../include/helper/server_helper.h"

using namespace std;

int main(void) {
  PlayersContainer punctuation;
  Player entry = punctuation.insertOrUpdatePlayer("DummyName", "address", 123);

  printf("%s\n", entry.toString().c_str());

  string a = "Hello";
  string b = "Hello";
  if (a == b) {
    printf("True\n");
  }
  
  return 0;
}