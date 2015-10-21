#include "db_manager.h"

Porter* port_init() {
  Porter* P = (Porter*)malloc(sizeof(Porter));
  P->file_d = open("item_list", O_RDWR);
  return P;
}

int port_read(const Porter* P, const int id, int* amount, int* price) {
  // check if that part of file is locked
  struct flock lock, savelock;
  lock.l_type   = F_WRLCK;
  lock.l_start  = (id-1)*sizeof(Item);
  lock.l_whence = SEEK_SET;
  lock.l_len    = sizeof(Item);
  fcntl(P->file_d, F_GETLK, &lock);

  if (lock.l_type == F_WRLCK) {
    return -1;
  }
  else {
    lseek(P->file_d, (id-1)*sizeof(Item), SEEK_SET);
    Item tmp;
    read(P->file_d, &tmp, sizeof(Item));
    *amount = tmp.amount;
    *price = tmp.price;
  }
  return 1;
}

int port_write(const Porter* P, const int id) {
  // check if that part of file is locked
  struct flock lock, savelock;
  lock.l_type   = F_WRLCK;
  lock.l_start  = (id-1)*sizeof(Item);
  lock.l_whence = SEEK_SET;
  lock.l_len    = sizeof(Item);
  lock.l_pid    = -1;
  savelock = lock;

  int result = fcntl(P->file_d, F_GETLK, &lock);
  if (lock.l_type == F_WRLCK) {
    return -1;
  }
  else {
    if (result == -1)
      return -1;
    else if (lock.l_pid != -1)
      return -1;
    else {
      savelock.l_pid = getpid();
      fcntl(P->file_d, F_SETLK, &savelock);
      return 1;
    }
  }
}

int port_unlock(const Porter* P, const int id) {
  struct flock lock;
  lock.l_type   = F_UNLCK;
  lock.l_start  = (id-1)*sizeof(Item);
  lock.l_whence = SEEK_SET;
  lock.l_len    = sizeof(Item);

  fcntl(P->file_d, F_SETLK, &lock);
}

int port_operate(const Porter* P, const char* input_buf, char* output_buf, const int id) {
  // parse the input command first
  char command[128];
  int number = -1;
  sscanf(input_buf, "%s %d", command, &number);

  if (number < 0){
    sprintf(output_buf,"Operation failed.\n");
    return 0;
  }

  // move offset to the specific position and read
  lseek(P->file_d, (id-1)*sizeof(Item), SEEK_SET);
  Item box;
  read(P->file_d, &box, sizeof(Item));

  // judge whether the operation is legal
  
  if (strcmp(command, "buy") == 0) {
    if (number > box.amount)
      sprintf(output_buf,"Operation failed.\n");
    else {
      // sprintf(output_buf,"usage: buy [amount]\n");
      box.amount -= number;
      lseek(P->file_d, (id-1)*sizeof(Item), SEEK_SET);
      write(P->file_d, &box, sizeof(Item));
      return 1;
    }
  }
  else if (strcmp(command, "sell") == 0) {
    if (number > box.amount)
      sprintf(output_buf,"Operation failed.\n");
    else {
      // sprintf(output_buf,"usage: sell [amount]\n");
      box.amount += number;
      lseek(P->file_d, (id-1)*sizeof(Item), SEEK_SET);
      write(P->file_d, &box, sizeof(Item));
      return 1;
    }
  }
  else if (strcmp(command, "price") == 0) {
    // sprintf(output_buf,"usage: price [new price]\n");
    box.price = number;
    lseek(P->file_d, (id-1)*sizeof(Item), SEEK_SET);
    write(P->file_d, &box, sizeof(Item));
    return 1;
  }
  else {
    sprintf(output_buf,"Operation failed.\n");
  }
  return 0;
}

int port_close(Porter* P) {
  close(P->file_d);
  return 1;
}

