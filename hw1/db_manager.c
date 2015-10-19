#include "db_manager.h"

Porter* port_init() {
  Porter* P = (Porter*)malloc(sizeof(Porter));
  P->file_d = open("item_list", O_RDWR);
  return P;
}

int db_list(const Porter* P) {
  lseek(P->file_d, 0, SEEK_SET);
  int i;
  Item tmp;
  for (i = 0; i < 20; ++i) {
    read(P->file_d, &tmp, sizeof(Item));
    printf("id = %d, amount = %d, price = %d\n", tmp.id, tmp.amount, tmp.price);
  }
  return 1;
}

int port_read(const Porter* P, const int id, int* amount, int* price) {
  lseek(P->file_d, (id-1)*sizeof(Item), SEEK_SET);
  Item tmp;
  read(P->file_d, &tmp, sizeof(Item));
  if (tmp.id == id) {
    *amount = tmp.amount;
    *price = tmp.price;
    return 1;
  }
  return -1;
}

int db_write(const Porter* P, const int id, const int amount, const int price) {
  lseek(P->file_d, (id-1)*sizeof(Item), SEEK_SET);
  Item tmp;
  read(P->file_d, &tmp, sizeof(Item));

  if (amount != -1)
    tmp.amount = amount;
  if (price != -1)
    tmp.price = price;
  printf("write to id = %d, amount = %d, price = %d\n", tmp.id, tmp.amount, tmp.price);
  lseek(P->file_d, (id-1)*sizeof(Item), SEEK_SET);
  write(P->file_d, &tmp, sizeof(Item));
  return 1;
}

int db_close(Porter* P) {
  close(P->file_d);
  return 1;
}
