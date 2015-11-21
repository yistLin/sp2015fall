#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>

void setUpFifoName(char fifoname[5][20], int host_id);

int main(int argc, char *argv[]) {

    if (argc != 2) {
        fputs("usage: ./host [host_id]", stderr);
        exit(0);
    }
    int host_id = atoi(argv[1]);

    int fifo_ptoh, fifo_htop[4];
    char fifoname[5][20];
    setUpFifoName(fifoname, host_id);
/*
    char buf[1024];
    memset(buf, 0, sizeof(char) * 1024);
    read(0, buf, 1024);

    fprintf(stderr, "HOST: id = %d. I read \"%s\" from STDIN\n", host_id, buf);

    memset(buf, 0, sizeof(char) * 1024);
    sprintf(buf, "this is host_%d with rand = %d", host_id, randnum);
    write(1, buf, strlen(buf));
*/
    return 0;
}

void setUpFifoName(char fifoname[5][20], int host_id) {
    struct stat st;
    sprintf(fifoname[0], "host%d.FIFO", host_id);
    if (stat(fifoname[0], &st) != 0)
        mkfifo(fifoname[0], 0666);

    char namepostfix[] = {'A', 'B', 'C', 'D'};
    for (int i = 1; i < 5; i++) {
        sprintf(fifoname[i], "host%d_%c.FIFO", host_id, namepostfix[i-1]);
        if (stat(fifoname[i], &st) != 0)
            mkfifo(fifoname[i], 0666);
    }
}

