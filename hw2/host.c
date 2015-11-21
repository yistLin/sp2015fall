#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>

void setUpFifoName(char fifoname[5][20], int host_id);

int main(int argc, char *argv[]) {

    if (argc != 2) {
        fputs("usage: ./host [host_id]", stderr);
        exit(-1);
    }
    int host_id = atoi(argv[1]);

    int fifo_ptoh, fifo_htop[4];
    char fifoname[5][20];
    setUpFifoName(fifoname, host_id);

    int num[4];
    char in_buf[1024];
    memset(in_buf, 0, sizeof(char) * 1024);
    while (1) {
        read(STDIN_FILENO, in_buf, 1024);
        sscanf(in_buf, "%d %d %d %d\n", &num[0], &num[1], &num[2], &num[3]);

        if (num[0] == -1 && num[1] == -1 && num[2] == -1 && num[3] == -1)
            break;

        printf("read: %d %d %d %d\n", num[0], num[1], num[2], num[3]);
    }
/*
    pid_t child;
    child = fork();

    if (child < 0) {
        perror("fork() error");
        exit(-1);
    }

    if (child != 0) {
        printf("I'm parent %d, and my child is %d\n", getpid(), child);
    } else {
        printf("I'm child\n");
        execlp("sleep", "sleep", "3", (char*)NULL);
        exit(0);
    }
*/
    int status;
    pid_t wpid;
    while (1) {
        wpid = wait(&status);
        if (wpid == -1 && errno == ECHILD)
            break;
    }

    for (int i = 0; i < 5; i++)
        unlink(fifoname[i]);

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

