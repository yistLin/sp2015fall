#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>

int main(void){
  char buf[128];
  fgets(buf, 128, stdin);
  fprintf(stdout, "STDOUT: %s", buf);
  fprintf(stderr, "STDERR: %s", buf);
  return 0;
}

