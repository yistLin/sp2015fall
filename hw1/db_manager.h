#ifndef DB_MANAGER_H_
#define DB_MANAGER_H_

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct {
  int id, amount, price;
} Item;

typedef struct {
  int file_d;
} Porter;

Porter* port_init();

int db_list(const Porter*);

int port_read(const Porter*, const int, int*, int*);

int db_write(const Porter*, const int, const int, const int);

int db_close(Porter*);

#endif // DB_MANAGER_H_
