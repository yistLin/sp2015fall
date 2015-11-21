#include <stdlib.h>

typedef struct node {
  int stat[4];
} Node;

typedef struct comb {
  Node *arr;
  int ptr;
  int end;
  int N;
} Comb;

void init_comb(Comb*, int);
void create_comb(Comb*);
void recur_get_comb(Comb*, int, int, int*);
void rewind_comb(Comb*);
int get_next_comb(Comb*, int*);
