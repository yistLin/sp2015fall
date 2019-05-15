#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#define SIZE 4096
int main(int argc, char* argv[]){
  char buf[SIZE];
  int n, i;
  char* str_out = "(stdout)\n";
  char* str_err = "(stderr)\n";
  while ((n = read(0, buf, SIZE)) > 0) {
    // remove the new line after input string
    n--;

    // write() to stderr, add "(stderr)\n" behind the input string
    for (i = 0; i < strlen(str_err); ++i)
      buf[n+i] = str_err[i];
    write(2, buf, n+strlen(str_err));
    
    // write() to stdout, add "(stdout)\n" behind the input string
    for (i = 0; i < strlen(str_out); ++i)
      buf[n+i] = str_out[i];
    write(1, buf, n+strlen(str_out));
  }
  return 0;
}
