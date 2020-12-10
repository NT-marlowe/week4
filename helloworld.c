#include <stdio.h>
#include <stdlib.h>

int main(void){
  float f[4];

  // 以下に適切な値を設定する
  f[0] = 0.0;
  f[1] = 1.0;
  f[2] = 2.0;
  f[3] = 3.0;

  // "Hello,World\n" と出力してほしい
  printf("%s",(char*)f);
  return 0;
}
