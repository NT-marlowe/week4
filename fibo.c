#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void pow_mat(int *a, int n);
int fibo(int *a, int n);

int main(int argc, char **argv) {
  if (argc == 1) {
    fprintf(stderr, "Enter int n\n");
    return EXIT_FAILURE;
  } 
  else if (argc > 2) {
    fprintf(stderr, "Too many arguments\n");
    return EXIT_FAILURE;
  } 
  else {
    int n = atoi(argv[1]);
    
    if (n < 0) {
      fprintf(stderr, "n should be 0 or positive\n");
      return EXIT_FAILURE;
    }
    
    int a[4] = {1,1,1,0};
    int ret = fibo(a,n);
    printf("fibo(%d) = %d\n", n, ret);
  }
  return EXIT_SUCCESS;
}

void pow_mat(int *a, int n) { //行列aに[[1,1],[1,0]]のn乗の値をセットする関数
  int tmp[4] = {0};
  for (int i = 0; i < 4; i++) tmp[i] = a[i];

  /*if (n == 0) {
    a[0] = 1;
    a[1] = 0;
    a[2] = 0;
    a[3] = 1;
  }*/
  
  if (n == 1) return;  //そのままでよい

  if (n % 2 == 0) {
    pow_mat(tmp, n/2);
    a[0] = tmp[0]*tmp[0] + tmp[1]*tmp[2];
    a[1] = tmp[0]*tmp[1] + tmp[1]*tmp[3];
    a[2] = tmp[2]*tmp[0] + tmp[3]*tmp[2];
    a[3] = tmp[2]*tmp[1] + tmp[3]*tmp[3]; 
  } 
  else {
    pow_mat(tmp, (n-1)/2);
    a[0] = tmp[0]*(tmp[0]+tmp[1]) + tmp[1]*(tmp[2]+tmp[3]);
    a[1] = tmp[0]*tmp[0] + tmp[1]*tmp[2];
    a[2] = tmp[2]*(tmp[0]+tmp[1]) + tmp[3]*(tmp[2]+tmp[3]);
    a[3] = tmp[2]*tmp[0] + tmp[3]*tmp[2]; 
  }
}

int fibo(int *a, int n) {
  if (n == 0 || n == 1) return 1;
  
  pow_mat(a, n-1);
  return a[0] + a[1];
}