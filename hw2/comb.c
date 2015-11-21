#include "comb.h"

void init_comb(Comb* c, int N) {
  c->N = N;
  int sum = 1;
  for (int i = N; i > N-4; i--)
    sum *= i;
  sum /= 24;
  c->ptr = 0;
  c->end = sum;
  c->arr = (Node*)malloc(sizeof(Node) * sum);
}

void create_comb(Comb *c) {
  int *stat = (int*)malloc(sizeof(int) * 4);
  recur_get_comb(c, 0, 0, stat);
  free(stat);
}

void recur_get_comb(Comb *c, int uptr, int sptr, int stat[]) {
  if (sptr == 4) {
    for (int i = 0; i < 4; i++)
      c->arr[c->ptr].stat[i] = stat[i];
    (c->ptr)++;
    return;
  }
  
  if (uptr >= c->N || (4 - sptr) > (c->N - uptr))
    return;
  else {
    stat[sptr] = uptr+1;
    recur_get_comb(c, uptr+1, sptr+1, stat);
    recur_get_comb(c, uptr+1, sptr, stat);
  }
}

void rewind_comb(Comb *c) {
  c->ptr = 0;
}

int get_next_comb(Comb *c, int arr[]) {
  if (c->ptr >= c->end)
    return 0;

  for (int i = 0; i < 4; i++)
    arr[i] = c->arr[c->ptr].stat[i];
  (c->ptr)++;
  return 1;
}

