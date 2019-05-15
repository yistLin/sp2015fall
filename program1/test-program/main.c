#include "db_manager.h"

int main(int argc, char const *argv[])
{
  Porter* P = db_init();
  db_list(P);
  printf("\n");

  db_read(P, 5);
  db_read(P, 19);
  db_read(P, 1);
  printf("\n");

  db_write(P, 1, 9, -1);
  db_write(P, 7, 5, 2000);

  db_list(P);
  db_close(P);
  return 0;
}