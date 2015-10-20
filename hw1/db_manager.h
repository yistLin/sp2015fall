#ifndef DB_MANAGER_H_
#define DB_MANAGER_H_

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int id, amount, price;
} Item;

typedef struct {
  int file_d;
} Porter;

Porter* port_init();

int port_read(const Porter*, const int, int*, int*);

int port_write(const Porter*, const int);

int port_unlock(const Porter*, const int);

int port_operate(const Porter*, const char*, char*, const int);

int port_close(Porter*);

#endif // DB_MANAGER_H_

