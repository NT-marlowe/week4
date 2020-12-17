#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h> // strtol のエラー判定用
#define INF 1e9

// 町の構造体（今回は2次元座標）を定義
typedef struct
{
  int x;
  int y;
} City;

// 描画用
typedef struct
{
  int width;
  int height;
  char **dot;
} Map;

typedef struct ans {
  double dist;
  int *route;
} Answer;

// 整数最大値をとる関数
int max(const int a, const int b)
{
  return (a > b) ? a : b;
}

double min(const double a, const double b) {
  return (a < b) ? a : b;
}

// プロトタイプ宣言
// draw_line: 町の間を線で結ぶ
// draw_route: routeでの巡回順を元に移動経路を線で結ぶ
// plot_cities: 描画する
// distance: 2地点間の距離を計算
// solve(): TSPをといて距離を返す/ 引数route に巡回順を格納

void draw_line(Map map, City a, City b);
void draw_route(Map map, City *city, int n, const int *route);
void plot_cities(FILE* fp, Map map, City *city, int n, const int *route);
double distance(City a, City b);
double solve(int n, double **dp, int **next_city, int bit, int v, double dist_table[n][n]);
void search_route(int n, int *route, int **next_city, int v, int bit, int idx);
Map init_map(const int width, const int height);
void free_map_dot(Map m);
City *load_cities(const char* filename,int *n);

Map init_map(const int width, const int height)
{
  char **dot = (char**) malloc(width * sizeof(char*));
  char *tmp = (char*)malloc(width*height*sizeof(char));
  for (int i = 0 ; i < width ; i++)
    dot[i] = tmp + i * height;
  return (Map){.width = width, .height = height, .dot = dot};
}
void free_map_dot(Map m)
{
  free(m.dot[0]);
  free(m.dot);
}

City *load_cities(const char *filename, int *n)
{
  City *city;
  FILE *fp;
  if ((fp=fopen(filename,"rb")) == NULL){
    fprintf(stderr, "%s: cannot open file.\n",filename);
    exit(1);
  }
  fread(n,sizeof(int),1,fp);
  city = (City*)malloc(sizeof(City) * *n);
  for (int i = 0 ; i < *n ; i++){
    fread(&city[i].x, sizeof(int), 1, fp);
    fread(&city[i].y, sizeof(int), 1, fp);
  }
  fclose(fp);
  return city;
}
int main(int argc, char**argv)
{
  // const による定数定義
  const int width = 70;
  const int height = 40;
  const int max_cities = 20; // 100から20に変更

  Map map = init_map(width, height);
  
  FILE *fp = stdout; // とりあえず描画先は標準出力としておく
  if (argc != 2){
    fprintf(stderr, "Usage: %s <city file>\n", argv[0]);
    exit(1);
  }
  int n;

  City *city = load_cities(argv[1],&n);
  assert( n > 1 && n <= max_cities); // さすがに都市数100は厳しいので
  
  double dist_table[n][n];
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      dist_table[i][j] = distance(city[i], city[j]); // bitDPのために距離のテーブルをセット
    } 
  }
  
  // 町の初期配置を表示
  plot_cities(fp, map, city, n, NULL);
  sleep(1);

  // 訪れる順序を記録する配列を設定
  int *route = (int*)calloc(n, sizeof(int));
  // 訪れた町を記録するフラグ
  // int *visited = (int*)calloc(n, sizeof(int));

  double **dp = (double**)calloc((1<<n)+1, sizeof(double*)); // {(1<<n) + 1} * (n+1)の二次元配列
  double *tmp = (double*)calloc((1<<n + 1) * (n + 1) , sizeof(double));
  
  int **next_city = (int**)calloc((1<<n)+1, sizeof(int*));
  int *tmp_city = (int*)calloc((1<<n + 1) * (n + 1), sizeof(int));
  
  for (int i = 0; i < (1<<n)+1; i++) {
    dp[i] = tmp + (n+1) * i;
    next_city[i] = tmp_city + (n+1) * i;
  }
  
  for (int i = 0; i < (1<<n); i++) {
    for (int j = 0; j < n; j++) dp[i][j] = -1; // 未探索のフラグ
  }

  double d = solve(n, dp, next_city, 0, 0, dist_table);
  search_route(n, route, next_city, 0, 0, 0);
  
  plot_cities(fp, map, city, n, route);
  printf("total distance = %f\n", d);
  for (int i = 0 ; i < n ; i++){
    printf("%d -> ", route[i]);
  }
  printf("0\n");

  // 動的確保した環境ではfreeをする
  free(route);
  // free(visited);
  free(city);

  free(tmp);
  free(dp);

  free(tmp_city);
  free(next_city);
  return 0;
}

// 繋がっている都市間に線を引く
void draw_line(Map map, City a, City b)
{
  const int n = max(abs(a.x - b.x), abs(a.y - b.y));
  for (int i = 1 ; i <= n ; i++){
    const int x = a.x + i * (b.x - a.x) / n;
    const int y = a.y + i * (b.y - a.y) / n;
    if (map.dot[x][y] == ' ') map.dot[x][y] = '*';
  }
}

void draw_route(Map map, City *city, int n, const int *route)
{
  if (route == NULL) return;

  for (int i = 0; i < n; i++) {
    const int c0 = route[i];
    const int c1 = route[(i+1)%n];// n は 0に戻る必要あり
    draw_line(map, city[c0], city[c1]);
  }
}

void plot_cities(FILE *fp, Map map, City *city, int n, const int *route)
{
  fprintf(fp, "----------\n");

  memset(map.dot[0], ' ', map.width * map.height); 

  // 町のみ番号付きでプロットする
  for (int i = 0; i < n; i++) {
    char buf[100];
    sprintf(buf, "C_%d", i);
    for (int j = 0; j < strlen(buf); j++) {
      const int x = city[i].x + j;
      const int y = city[i].y;
      map.dot[x][y] = buf[j];
    }
  }

  draw_route(map, city, n, route);

  for (int y = 0; y < map.height; y++) {
    for (int x = 0; x < map.width; x++) {
      const char c = map.dot[x][y];
      fputc(c, fp);
    }
    fputc('\n', fp);
  }
  fflush(fp);
}

double distance(City a, City b)
{
  const double dx = a.x - b.x;
  const double dy = a.y - b.y;
  return sqrt(dx * dx + dy * dy);
}

double solve(int n, double **dp, int **next_city, int bit, int v, double dist_table[n][n])
{
  if (dp[bit][v] >= 0) return dp[bit][v];

  if (bit == (1<<n)-1 && v == 0) return dp[bit][v] = 0;

  // int prev_bit = bit & ~(1<<v);

  double ret = INF;

  for (int u = 0; u < n; u++) {
    if (!(bit >> u & 1)) {
      // 次の頂点はu
      int next = bit | (1 << u);
      double now = solve(n, dp, next_city, next, u, dist_table) + dist_table[u][v];
      // ret = min(ret, now);
      if (ret > now) {
        ret = now;
        next_city[bit][v] = u;
      }
    }
  }
  return dp[bit][v] = ret;
 
}

void search_route(int n, int *route, int **next_city, int v, int bit, int idx) {
  
  if (idx == n-1) return;

  route[idx+1] = next_city[bit][v];
  int nv = route[idx+1];
  int next_bit = bit | (1 << nv);
  printf("%d ", nv);
  search_route(n, route, next_city, nv, next_bit, idx+1);
}