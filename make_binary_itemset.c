#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 課題3のテスト用プログラム
// アイテムセットのバイナリファイルを書き出す
int main() {
  char *filename = "binary_item.txt";
  FILE *fp = fopen(filename, "wb");

  int n = 10;
  fwrite(&n, sizeof(int), 1, fp);

  double value, weight;
  for (int i = 0; i < n; i++) {
    value = 0.1 * (rand() % 200);
    fwrite(&value, sizeof(double), 1, fp);
  }
  for (int i = 0; i < n; i++) {
    weight = 0.1 * (rand() % 200);
    fwrite(&weight, sizeof(double), 1, fp);
  }
  
  return 0;
}