#include <stdio.h>
#include <stdlib.h>

int main(int argc, char**argv)
{
  // 1000万作成したdouble を テキスト/バイナリファイルで出力してみる
  // 先頭に個数情報を出力しておく
  // バイナリの場合はsizeof(size_t)分を最初に出力する
  if (argc != 3){
    fprintf(stderr,"usage: %s <txt filename> <binary filename>\n",argv[1]);
    return EXIT_FAILURE;
  }
  size_t size = 10000000;
  double *d = (double*)malloc(sizeof(double)*size); // とりあえず1000万確保

  srand(100);
  for(int i = 0 ; i < size ; i++)
    d[i] = 0.5423 * rand();

  //テキストに出力してみる
  FILE *fp;
  if ( (fp = fopen(argv[1],"w")) == NULL ){
    fprintf(stderr,"%s: cannot open file.\n",argv[1]);
    return EXIT_FAILURE;
  }

  fprintf(fp, "%zu\n",size);
  for(int i = 0 ; i < size ; i++)
    fprintf(fp,"%f\n",d[i]);
  fclose(fp);

  // "wb"の'b'はwindows は必須, linux/mac はなくてもよい ("w" でOK)
  if ( (fp = fopen(argv[2],"wb")) == NULL ){
    fprintf(stderr,"%s: cannot open file.\n",argv[1]);
    return EXIT_FAILURE;  
  }
  // 最初にsizeof(size_t) 1個分をサイズ情報として出しておく
  fwrite(&size,sizeof(size_t),1,fp);
  // dポインタを先頭にからsize個のdoubleデータを出力
  fwrite(d,sizeof(double),size,fp);
  fclose(fp);
  return EXIT_SUCCESS;
}
