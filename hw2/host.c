#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

    if (argc != 2) {
        fputs("usage: ./host [host_id]", stderr);
        exit(0);
    }

    int host_id = atoi(argv[1]);

    char buf[1024];
    memset(buf, 0, sizeof(char) * 1024);
    read(0, buf, 1024);

    fprintf(stderr, "HOST: id = %d. I read \"%s\" from STDIN\n", host_id, buf);

    srand((unsigned int) time(NULL));

    sleep(host_id);

    int randnum = 0;
    for (int i = 0; i < host_id; i++)
        randnum = (int)rand() % 100;

    memset(buf, 0, sizeof(char) * 1024);
    sprintf(buf, "this is host_%d with rand = %d", host_id, randnum);
    write(1, buf, strlen(buf));

    return 0;
}

