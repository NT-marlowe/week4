#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h> // strtol のエラー判定用
#include <time.h>

#define INF 1e9 // 最短距離の解の初期値

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

int min(const int a, const int b) {
  return (a < b) ? a : b;
}

void swap(int *a, int *b) {
  int tmp = *a;
  *a = *b;
  *b = tmp;
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
double total_distance(const City *city, int *route, int n);
void gen_random_permutation(int *pattern, int n);
Answer yamanobori(const City *city, int *route, int n);
Answer solve(const City *city, int n);
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
  if ((fp = fopen(filename,"rb")) == NULL){
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
  srand(time(NULL));
  
  
  // const による定数定義
  const int width = 70;
  const int height = 40;
  const int max_cities = 100;

  Map map = init_map(width, height);
  
  FILE *fp = stdout; // とりあえず描画先は標準出力としておく
  if (argc != 2){
    fprintf(stderr, "Usage: %s <city file>\n", argv[0]);
    exit(1);
  }
  int n;

  City *city = load_cities(argv[1],&n);
  assert( n > 1 && n <= max_cities); // さすがに都市数100は厳しいので
  // 町の初期配置を表示
  // plot_cities(fp, map, city, n, NULL);
  sleep(1);

  // 訪れる順序を記録する配列を設定
  int *route = (int*)calloc(n, sizeof(int));
  // 訪れた町を記録するフラグ
  // int *visited = (int*)calloc(n, sizeof(int));

  Answer ans = (Answer){ .dist = INF, .route = route};
  Answer tmp;
  for (int i = 0; i < 1e4; i++) {// とりあえず初期解5個
    tmp = solve(city, n);
    if (ans.dist > tmp.dist) {
      ans.dist = tmp.dist;
      memcpy(ans.route, tmp.route, sizeof(int) * n);
    }
    free(tmp.route);
  }
  
  // plot_cities(fp, map, city, n, ans.route);
  printf("total distance = %f\n", ans.dist);
  for (int i = 0 ; i < n ; i++){
    printf("%d -> ", ans.route[i]);
  }
  printf("0\n");

  // 動的確保した環境ではfreeをする
  free(route);
  // free(visited);
  free(city);
  
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

void gen_random_permutation(int *pattern, int n) {
  // srand(time(NULL));
  pattern[0] = 0;

  int num = n-1;
  int stock[num];
  for (int i = 0; i < num; i++) {
    stock[i] = i+1;
  }
  
  for (int i = 0; i < num; i++) {
    int idx = rand() % (num-i);
    pattern[i+1] = stock[idx];

    int tmp = stock[idx];
    stock[idx] = stock[num-i-1];
    stock[num-i-1] = tmp;
  }
}

double total_distance(const City *city, int *route, int n) {
  double sum = 0;
  for (int i = 0; i < n; i++) {
    const int c0 = route[i];
    const int c1 = route[(i+1)%n];
    sum += distance(city[c0], city[c1]);
  }
  return sum;
}

Answer yamanobori(const City *city, int *route, int n) {
  
  // 渡された配列の近傍を取る前に、距離を計算しておく
  int *arg = (int*)calloc(n, sizeof(int));
  memcpy(arg, route, sizeof(int) * n);
  Answer origin = (Answer){ .dist = 0, .route = arg};
  origin.dist = total_distance(city, route, n);

  int hamming2_route[n];
  // memcpy(hamming2_route, route, sizeof(int) * n);
  for (int i = 0; i < n; i++) hamming2_route[i] = route[i]; 
  
  // ハミング距離2のルートの距離を計算し、短いものがあれば更新する
  for (int i = 1; i < n-1; i++) {
    for (int j = i+1; j < n; j++) {
      swap(&hamming2_route[i], &hamming2_route[j]);
      // int now = hamming2_route[i];
      // hamming2_route[i] = hamming2_route[j];
      // hamming2_route[j] = now;
      
      double tmp = total_distance(city, hamming2_route, n);
      if (origin.dist > tmp) {
        origin.dist = tmp;
        memcpy(origin.route, hamming2_route, sizeof(int) * n);
      }
      swap(&hamming2_route[i], &hamming2_route[j]);
      // now = hamming2_route[i];
      // hamming2_route[i] = hamming2_route[j];
      // hamming2_route[j] = now;
    }
  }
  return origin;
}

Answer solve(const City *city, int n) { //山登り法としてはsolveだが、最適解を求めてはいない
  
  int *route = (int*)calloc(n, sizeof(int)); //137行目でfreeしている
  
  gen_random_permutation(route, n); //初期解をセット
  double origin_distance = total_distance(city, route, n);
  
  Answer ans = (Answer){ .dist = INF, .route = NULL};
  while (1) {
    ans = yamanobori(city, route, n);
    memcpy(route, ans.route, sizeof(int) * n);
    if (ans.dist == origin_distance) {
      return ans;  //解が更新されなくなったら終了
    } 
    else {
      origin_distance = ans.dist;
      free(ans.route);
    }
  }

  return ans; 
}

/*Answer solve(const City *city, int n, int *route, int *visited, int visited_number)
{
  // 以下はとりあえずダミー。ここに探索プログラムを実装する
  // 現状は町の番号順のルートを回っているだけ
  // 実際は再帰的に探索して、組み合わせが膨大になる。
  route[0] = 0; // 循環した結果を避けるため、常に0番目からスタート
  visited[0] = 1;
  
  if (visited_number == n) {
    double sum_d = 0;
    for (int i = 0 ; i < n ; i++){
      const int c0 = route[i];
      const int c1 = route[(i+1)%n]; // nは0に戻る
      sum_d += distance(city[c0],city[c1]);
    }
    int *arg = (int*)malloc(sizeof(int) * n);
    memcpy(arg, route, sizeof(int) * n);
    return (Answer){ .dist = sum_d, .route = arg};
  }

  
  // トータルの巡回距離を計算する
  // 実際には再帰の末尾で計算することになる
  /*double sum_d = 0;
  for (int i = 0 ; i < n ; i++){
    const int c0 = route[i];
    const int c1 = route[(i+1)%n]; // nは0に戻る
    sum_d += distance(city[c0],city[c1]);
  }*/
  
