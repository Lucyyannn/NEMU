#include <stdio.h>
#include <assert.h>

int main() {
  FILE *fp = fopen("/share/texts/num", "r+");
  assert(fp);

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  assert(size == 5000);

  printf("how are you 111\n");

  fseek(fp, 500 * 5, SEEK_SET);
  printf("how are you 222\n");
  long seek1 = ftell(fp);
  printf("seek1: %d\n",seek1);
  int n2;
  fscanf(fp, "%d", &n2);
  printf("n2 at 2500: %d\n",n2);

  int i, n;
  for (i = 500; i < 1000; i ++) {
    printf("how are you 333\n");
    fscanf(fp, "%d", &n);
    printf("how are you 444\n");
    printf("i= %d, n= %d \n",i,n);
    assert(n == i + 1);
  }
  printf("how are you 555\n");

  fseek(fp, 0, SEEK_SET);
  for (i = 0; i < 500; i ++) {
    fprintf(fp, "%4d\n", i + 1 + 1000);
  }

  for (i = 500; i < 1000; i ++) {
    fscanf(fp, "%d", &n);
    assert(n == i + 1);
  }

  fseek(fp, 0, SEEK_SET);
  for (i = 0; i < 500; i ++) {
    fscanf(fp, "%d", &n);
    assert(n == i + 1 + 1000);
  }

  fclose(fp);

  printf("PASS!!!\n");

  return 0;
}
