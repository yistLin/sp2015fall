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

typedef struct bit {
    int money;
    char index;
} Bit;

typedef struct record {
    int pindex;
    int score;
} Record;

int cmpfunc(const void* a, const void* b) {
    return ((Bit*)b)->money - ((Bit*)a)->money;
}

int cmpfunc2(const void* a, const void* b) {
    return ((Record*)b)->score - ((Record*)a)->score;
}

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
    char rtn_msg[30], rtn_msg_cpy[30];
    char rtn_index;
    int rtn_hash, rtn_money;
    Bit bit[4];
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

        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 4; j++)
                playerMoney[j] += 1000;

            memset(cmpt_msg, 0, sizeof(char) * 30);
            sprintf(cmpt_msg, "%d %d %d %d\n", playerMoney[0], playerMoney[1], playerMoney[2], playerMoney[3]);
            msg_len = strlen(cmpt_msg);

            for (int j = 0; j < 4; j++)
                write(fifo_htop[j], cmpt_msg, msg_len);

            memset(rtn_msg_cpy, 0, sizeof(char) * 30);
            for (int j = 0; j < 4; j++) {
                fgets(rtn_msg, 30, fifo_ptoh);
                if (strcmp(rtn_msg, rtn_msg_cpy) == 0) {
                    j--;
                    continue;
                }
                sscanf(rtn_msg, " %c %d %d\n", &rtn_index, &rtn_hash, &rtn_money);
                bit[j].index = rtn_index;
                bit[j].money = rtn_money;
                // fprintf(stderr, "from player%c : hash = %d, money = %d\n", rtn_index, rtn_hash, rtn_money);
                strcpy(rtn_msg, rtn_msg_cpy);
            }

            qsort(bit, 4, sizeof(Bit), cmpfunc);
            // for (int j = 0; j < 4; j++) fprintf(stderr, "player%c: %d\n", bit[j].index, bit[j].money);

            char winner = '0';
            int hiprice;
            int conflictFlag = 0;
            for (int j = 1; j < 4; j++) {
                if (bit[j].money == bit[j-1].money) {
                    conflictFlag = 1;
                }
                else if (bit[j].money != bit[j-1].money && conflictFlag != 1) {
                    winner = bit[j-1].index;
                    hiprice = bit[j-1].money;
                    break;
                }
                else
                    conflictFlag = 0;
            }
            if (winner == '0' && conflictFlag == 0) {
                winner = bit[3].index;
                hiprice = bit[3].money;
            }

            // fprintf(stderr, "winner is %c\n", winner);

            if (winner != '0') {
                playerMoney[winner - 'A'] -= hiprice;
                playerScore[winner - 'A']++;
            }

            // for (int j = 0; j < 4; j++) fprintf(stderr, "player%c : money = %d, score = %d\n", playerIndex[j], playerMoney[j], playerScore[j]);
        }

        // after all thing done, close something
        int status;
        pid_t wpid;
        while ((wpid = wait(&status)) > 0);
        fclose(fifo_ptoh);
        for (int i = 0; i < 4; i++) close(fifo_htop[i]);

        Record records[4];
        for (int i = 0; i < 4; i++) {
            records[i].pindex = i;
            records[i].score = playerScore[i];
        }
        qsort(records, 4, sizeof(Record), cmpfunc2);

        // for (int i = 0; i < 4; i++) {
        //     fprintf(stderr, "#%d player%c: id = %d, score = %d\n", i+1, (char)('A'+records[i].pindex), playerID[records[i].pindex], records[i].score);
        // }

        int rank[4];
        int tmp_rank = 1, accumula = 1;
        rank[ records[0].pindex ] = 1;
        for (int i = 1; i < 4; i++) {
            if ( records[i].score == records[i-1].score ) {
                rank[ records[i].pindex ] = tmp_rank;
                accumula++;
            } else {
                tmp_rank += accumula;
                rank[ records[i].pindex ] = tmp_rank;
                accumula = 1;
            }
        }

        for (int i = 0; i < 4; i++)
            printf("%d %d\n", playerID[i], rank[i]);
        fflush(stdout);
    } //  END of while loop

    for (int i = 0;i < 5; i++) unlink(fifoname[i]);

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

