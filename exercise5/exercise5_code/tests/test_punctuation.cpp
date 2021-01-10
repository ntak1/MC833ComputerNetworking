#include <cstdio>
#include "../include/helper/server_helper.h"

using namespace std;

int main(void) {
  Punctuation punctuation;
  Player entry = punctuation.insertPlayer("DummyName", "address", 123);

  printf("%s\n", entry.toString().c_str());

  string a = "Hello";
  string b = "Hello";
  if (a == b) {
    printf("True\n");
  }
  
  return 0;
}