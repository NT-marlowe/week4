#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
  FILE *fp;
  char *filename = "helloworld.txt";
  if ((fp = fopen(filename, "rb")) == NULL) {
    perror(filename);
    return EXIT_FAILURE;
  }

  float f[4] = {0};
  size_t rsize = fread(f, sizeof(float), 4, fp);
  for (int i = 0; i < 4; i++) {
    printf("%.20e\n", f[i]);
  }
  fclose(fp);
  printf("%zu\n", rsize);
  return 0;
}