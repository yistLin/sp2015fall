#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
int main(int argc, char *argv[]){
  int fd1 = open("infile", O_RDONLY);
  dup2(fd1, 0);
  close(fd1);

  dup2(1, 2);

  int fd2 = open("outfile", O_WRONLY | O_CREAT, 0666);
  dup2(fd2, 1);
  close(fd2);

  char buf[128];
  fgets(buf, 128, stdin);
  fprintf(stdout, "STDOUT: %s", buf);
  fprintf(stderr, "STDERR: %s", buf);
  return 0;
}
