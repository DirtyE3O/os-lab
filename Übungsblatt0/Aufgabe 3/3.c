#include <stdio.h>

int main() {
  int table[10][10];
  for(int a = 0; a < 10; a++){
    for(int b = 0; b < 10; b++){
      table[a][b] = (a + 1) * (b + 1);
    }
  }

  for(int a = 0; a < 10; a++){
    for(int b = 0; b < 10; b++){
      printf("%d ", table[a][b]);
    }
    printf("\n");
  }
  return 0;
}