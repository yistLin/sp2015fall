#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
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

    int fifo_htop[4];
    FILE* fifo_ptoh;
    char fifoname[5][20];
    setUpFifoName(fifoname, host_id);

    char in_buf[1024];
    memset(in_buf, 0, sizeof(char) * 1024);

    srand((unsigned int) time(NULL));
    char playerIndex[] = {'A', 'B', 'C', 'D'};
    char params[3][10];
    pid_t childPID;
    int playerID[4];
    int playerHash[4];
    int playerMoney[4];
    int playerScore[4];

    char cmpt_msg[30];
    char rtn_msg[30];
    char rtn_index;
    int rtn_hash, rtn_money;
    int msg_len;

    while (1) {
        read(STDIN_FILENO, in_buf, 1024);
        sscanf(in_buf, "%d %d %d %d\n", &playerID[0], &playerID[1], &playerID[2], &playerID[3]);

        if (playerID[0] == -1 && playerID[1] == -1 && playerID[2] == -1 && playerID[3] == -1)
            break;
        
        for (int i = 0; i < 4; i++) {
            // fork child process now
            playerHash[i] = rand() % 65536;
            playerMoney[i] = playerScore[i] = 0;

            childPID = fork();
            if (childPID < 0) {
                perror("fork() error");
                exit(-1);
            }

            if (childPID == 0) {
                memset(params[0], 0, sizeof(char) * 10);
                memset(params[1], 0, sizeof(char) * 10);
                memset(params[2], 0, sizeof(char) * 10);
                sprintf(params[0], "%d", host_id);
                sprintf(params[1], "%c", playerIndex[i]);
                sprintf(params[2], "%d", playerHash[i]);
                execlp("./player", "./player", params[0], params[1], params[2], (char*)NULL);
                _exit(EXIT_FAILURE);
            }
            else {
                fifo_htop[i] = open(fifoname[i+1], O_WRONLY | O_TRUNC);
            }
        }

        truncate(fifoname[0], 0);
        fifo_ptoh = fopen(fifoname[0], "r");
        // fifo_ptoh = open(fifoname[0], O_RDONLY | O_TRUNC);

        for (int i = 0; i < 1; i++) {
            for (int j = 0; j < 4; j++)
                playerMoney[j] += 1000;

            memset(cmpt_msg, 0, sizeof(char) * 30);
            sprintf(cmpt_msg, "%d %d %d %d\n", playerMoney[0], playerMoney[1], playerMoney[2], playerMoney[3]);
            msg_len = strlen(cmpt_msg);

            for (int j = 0; j < 4; j++) {
                write(fifo_htop[j], cmpt_msg, msg_len);
            }

            memset(rtn_msg, 0, sizeof(char) * 30);
            while (fgets(rtn_msg, 30, fifo_ptoh) != NULL) {
                sscanf(rtn_msg, " %c %d %d\n", &rtn_index, &rtn_hash, &rtn_money);
                fprintf(stderr, "from player%c : hash = %d, money = %d\n", rtn_index, rtn_hash, rtn_money);
                memset(rtn_msg, 0, sizeof(char) * 30);
            }
        }

        // after all thing done, close something
        int status;
        pid_t wpid;
        while ((wpid = wait(&status)) > 0);
        fclose(fifo_ptoh);
        for (int i = 0; i < 4; i++) close(fifo_htop[i]);
    } //  END of while loop

    for (int i = 0; i < 5; i++) unlink(fifoname[i]);

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

