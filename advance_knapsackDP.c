#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

typedef struct item
{
  double value;
  double weight;
}Item;


typedef struct itemset
{
  int number;
  Item *item;
} Itemset;

Itemset *init_itemset(int number, int seed);

void free_itemset(Itemset *list);

Itemset *load_itemset(char *filename);

void print_itemset(const Itemset *list);

void save_itemset(char *filename);

void solve(const Itemset *list, int n, int capacity, double **dp, int **a);

void search_flags(int idx, int weight, const Itemset *list, int **a, int *flags);

// エラー判定付きの読み込み関数
int load_int(const char *argvalue);
double load_double(const char *argvalue);

int load_int(const char *argvalue)
{
  long nl;
  char *e;
  errno = 0; // errno.h で定義されているグローバル変数を一旦初期化
  nl = strtol(argvalue,&e,10);
  if (errno == ERANGE){
    fprintf(stderr,"%s: %s\n",argvalue,strerror(errno));
    exit(1);
  }
  if (*e != '\0'){
    fprintf(stderr,"%s: an irregular character '%c' is detected.\n",argvalue,*e);
    exit(1);
  }
  return (int)nl;
}

double load_double(const char *argvalue)
{
  double ret;
  char *e;
  errno = 0; // errno.h で定義されているグローバル変数を一旦初期化
  ret = strtod(argvalue,&e);
  if (errno == ERANGE){
    fprintf(stderr,"%s: %s\n",argvalue,strerror(errno));
    exit(1);
  }
  if (*e != '\0'){
    fprintf(stderr,"%s: an irregular character '%c' is detected.\n",argvalue,*e);
    exit(1);
  }
  return ret;
}

double max(double a, double b) {
  return (a > b) ? a : b;
}

int main(int argc, char **argv) {
  
  if (argc != 3){ // main, filename, capacity
    // fprintf(stderr, "usage: %s <the number of items (int)> <max capacity (double)>\n",argv[0]);
    fprintf(stderr, "usage: %s <the filename which sets the itemset> <max capacity (double)>\n",argv[0]);
    exit(1);
  }

  char *filename = argv[1];
  FILE *fp;
  if ((fp = fopen(filename, "rb")) == NULL) {
    perror(filename);
    return EXIT_FAILURE;
  }

  Itemset *list = load_itemset(filename);
  int n = list->number;

  const double W = load_double(argv[2]);
  assert( W >= 0.0);
  printf("max capacity: W = %.f, # of items: %d\n", W, n);
  
  int capacity = (int)(W * 10 + 0.1);

  double **dp = (double**)calloc(n+10, sizeof(double*)); //　itemnum * maxweight
  double *tmp = (double*)calloc((n+10)*(capacity+10), sizeof(double)); //　少し余裕をもたせる
  int **a = (int**)calloc(n+10, sizeof(int*));
  int *tmp_a = (int*)calloc((n+10)*(capacity+10), sizeof(int));
  
  for (int i = 0; i < n+10; i++) {
    dp[i] = tmp + i * (capacity+10);
    a[i] = tmp_a + i * (capacity+10);
  }
  
  int flags[n]; // 容量が小さいのでmallocしなくてよい
  solve(list, n, capacity, dp, a);
  search_flags(n, capacity, list, a, flags);

  for (int i = 0; i < n; i++) {
    printf("%.1f %.1f\n", list->item[i].value, list->item[i].weight);
  }

  printf("----\nbest solution:\n");
  for (int i = 0; i < n; i++) {
    printf("%d", flags[i]);
  }
  printf("\n");
  printf("value: %4.1f\n", dp[n][capacity]);

  free_itemset(list);
  free(tmp);
  free(dp);
  free(tmp_a);
  free(a);

  return 0;
}

Itemset *load_itemset(char *filename) {
  FILE *fp = fopen(filename, "rb");
  assert(fp != NULL);

  Itemset *list = (Itemset*)malloc(sizeof(Itemset));

  size_t rsize = fread(&(list->number), sizeof(int), 1, fp);
  if (rsize != 1) {
    fprintf(stderr, "%s is invalid file.\n", filename);
    exit(1);
  }; //ファイルの形式が誤っている

  int number = list->number;
  Item *item = (Item*)malloc(sizeof(Item) * number);

  double val[number], wei[number];
  size_t rsize_v = fread(val, sizeof(double), number, fp);
  size_t rsize_w = fread(wei, sizeof(double), number, fp);

  if (rsize_v != number || rsize_v != rsize_w) {
    fprintf(stderr, "%s is invalid file.\n", filename);
    exit(1);
  }  
  
  for (int i = 0; i < number; i++) {
    item[i].value = val[i];
    item[i].weight = wei[i];
  }
  
  *list = (Itemset){ .number = number, .item = item};
}

// itemset の free関数
void free_itemset(Itemset *list)
{
  free(list->item);
  free(list);
}

void solve(const Itemset *list, int n, int capacity, double **dp, int **a) {
  
  double v[n];
  int w[n];
  for (int i = 0; i < n; i++) {
    v[i] = list->item[i].value;
    w[i] = (int)(list->item[i].weight * 10 + 0.1); //　小数点1桁までしかないので10倍してintにキャストする
  }
  
  for (int i = 0; i <= n; i++) {
    for (int j = 0; j <= capacity; j++) {
      dp[i][j] = 0;
    }
  }
  
  for (int i = 0; i < n; i++) {
    for (int j = 0; j <= capacity; j++) {
      if (w[i] <= j) {
        // dp[i+1][j] = max(dp[i][j], dp[i][j-w[i]] + v[i]);
        if (dp[i][j] < dp[i][j-w[i]] + v[i]) {
          dp[i+1][j] = dp[i][j-w[i]] + v[i];
          a[i+1][j] = 1; // i番目のitemを採用した
        }
        else {
          dp[i+1][j] = dp[i][j];
          a[i+1][j] = 0; // 採用しなかった
        }
      } 
      else {
        dp[i+1][j] = dp[i][j];
        a[i+1][j] = 0;
      }
    }
  }
}

void search_flags(int idx, int weight, const Itemset *list, int **a, int *flags) {
  if (idx == 0) {return;}
  
  flags[idx-1] = a[idx][weight]; 
  // printf("%d ", weight);
  if (a[idx][weight]) {
    search_flags(idx-1, (int)(weight-list->item[idx-1].weight*10), list, a, flags);
  }
  else {
    search_flags(idx-1, weight, list, a, flags);
  }
} 