#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>

#include "comb.h"

typedef struct pc {
    int index, score;
} PC;

int cmpfunc(const void *a, const void *b) {
    return ((PC*)b)->score - ((PC*)a)->score;
}

int** dim2array(int, int);
void fork_host(int, int**, int**);

int main(int argc, char *argv[]) {
    
    if (argc != 3) {
        fputs("usage: ./bidding_system [host_num] [player_num]", stderr);
        exit(0);
    }

    int host_num = atoi(argv[1]);
    int player_num = atoi(argv[2]);
    if (host_num < 1 || host_num > 12) {
        fprintf(stderr, "host_num = %d. It should be 1 ≤ host_num ≤ 12\n", host_num);
        exit(0);
    }
    if (player_num < 4 || player_num > 20) {
        fprintf(stderr, "player_num = %d. It should be 4 ≤ player_num ≤ 20\n", player_num);
        exit(0);
    }

    int **pfd_stoh = dim2array(host_num, 2);
    int **pfd_htos = dim2array(host_num, 2);
    for (int i = 0; i < host_num; i++) {
        pipe(pfd_stoh[i]);
        pipe(pfd_htos[i]);
    }

    fork_host(host_num, pfd_stoh, pfd_htos);

    for (int i = 0; i < host_num; i++) {
        close(pfd_htos[i][1]);
        close(pfd_stoh[i][0]);
    }

    Comb *c = (Comb*)malloc(sizeof(Comb));
    init_comb(c, player_num);
    create_comb(c);
    rewind_comb(c);

    fd_set readset, allset;
    FD_ZERO(&allset);
    int max_fd = 0;

    for (int i = 0; i < host_num; i++) {
        FD_SET(pfd_htos[i][0], &allset);
        if (pfd_htos[i][0] > max_fd)
            max_fd = pfd_htos[i][0];
    }

    int comb[4];
    char buf[1024], w_buf[20];
    char *end_msg = "-1 -1 -1 -1\n";
    memset(buf, 0, sizeof(char) * 1024);
    memset(w_buf, 0, sizeof(char) * 20);

    int end_flag = 0;
    int counter = host_num;

    for (int i = 0; i < host_num; i++) {
        if (end_flag == 1) {
            write(pfd_stoh[i][1], end_msg, strlen(end_msg));
            FD_CLR(pfd_htos[i][0], &allset);
            counter--;
            continue;
        }

        if (get_next_comb(c, comb) == 1) {
            sprintf(w_buf, "%d %d %d %d\n", comb[0], comb[1], comb[2], comb[3]);
            write(pfd_stoh[i][1], w_buf, strlen(w_buf));
            memset(w_buf, 0, sizeof(char) * 20);
        }
        else {
            end_flag = 1;
            write(pfd_stoh[i][1], end_msg, strlen(end_msg));
            FD_CLR(pfd_htos[i][0], &allset);
            counter--;
        }
    }

    PC *result = (PC*)malloc( sizeof(PC) * (player_num+1) );
    for (int i = 0; i <= player_num; i++) {
        result[i].index = i;
        result[i].score = 0;
    }
    int tmp_player[4];
    int tmp_rank[4];

    while (1) {
        readset = allset;
        select(max_fd + 1, &readset, NULL, NULL, NULL);

        for (int i = 0; i < host_num; i++) {
            if (FD_ISSET(pfd_htos[i][0], &readset)) {
                read(pfd_htos[i][0], buf, 1024);
                // fprintf(stderr, "SYSTEM: from host_%d:\n", i+1);
                // fprintf(stderr, buf);
                sscanf(buf, "%d %d\n%d %d\n%d %d\n%d %d\n",
                        &tmp_player[0], &tmp_rank[0],
                        &tmp_player[1], &tmp_rank[1],
                        &tmp_player[2], &tmp_rank[2],
                        &tmp_player[3], &tmp_rank[3]);
                for (int j = 0; j < 4; j++)
                    result[ tmp_player[j] ].score += (4 - tmp_rank[j]);

                if (end_flag) {
                    write(pfd_stoh[i][1], end_msg, strlen(end_msg));
                    FD_CLR(pfd_htos[i][0], &allset);
                    counter--;
                }
                else {
                    if (get_next_comb(c, comb)) {
                        sprintf(w_buf, "%d %d %d %d\n", comb[0], comb[1], comb[2], comb[3]);
                        write(pfd_stoh[i][1], w_buf, strlen(w_buf));
                        memset(w_buf, 0, sizeof(char) * 20);
                    }
                    else {
                        end_flag = 1;
                        write(pfd_stoh[i][1], end_msg, strlen(end_msg));
                        FD_CLR(pfd_htos[i][0], &allset);
                        counter--;
                    }
                }
            }
        } // end loop select
        if (counter <= 0)
            break;

    } // end while

    // for (int i = 1; i <= player_num; i++)
    //     fprintf(stdout, "player%d : %d points\n", result[i].index, result[i].score);

    PC *result_sorted = (PC*)malloc(sizeof(PC) * (player_num+1));
    for (int i = 1; i <= player_num; i++) {
        result_sorted[i].index = result[i].index;
        result_sorted[i].score = result[i].score;
    }

    qsort(result_sorted+1, player_num, sizeof(PC), cmpfunc);

    int rank = 1;
    int rank_accumulative = 1;
    result[ result_sorted[1].index ].score = 1;
    for (int i = 2; i <= player_num; i++) {
        if (result_sorted[i].score == result_sorted[i-1].score) {
            result[ result_sorted[i].index ].score = rank;
            rank_accumulative++;
        }
        else {
            rank += rank_accumulative;
            rank_accumulative = 1;
            result[ result_sorted[i].index ].score = rank;
        }
    }

    for (int i = 1; i <= player_num; i++)
        fprintf(stdout, "%d %d\n", result[i].index, result[i].score);

    free(pfd_stoh);
    free(pfd_htos);
    free(c);
    free(result);

    return 0;
}

int** dim2array(int m, int n) {
    int **array, *pData;
    array = (int**)malloc(m * sizeof(int*));
    pData = (int*)malloc(n * m * sizeof(int));
    for (int i = 0; i < m; i++, pData += 2)
        array[i] = pData;
    return array;
}

void fork_host(int host_num, int** pfd_stoh, int** pfd_htos) {
    pid_t pid;
    char hostid[3];
    for (int i = 0; i < host_num; i++) {
        pid = fork();
        if (pid < 0) {
            fputs("fork error", stderr);
            exit(0);
        }
        
        if (pid == 0) {
            // In child process
            dup2(pfd_stoh[i][0], 0);
            dup2(pfd_htos[i][1], 1);
            for (int j = 0; j < host_num; j++) {
                close(pfd_stoh[j][0]);
                close(pfd_stoh[j][1]);
                close(pfd_htos[j][0]);
                close(pfd_htos[j][1]);
            }
            sprintf(hostid, "%d", i+1);
            execlp("./host", "hw2/host", hostid, (char*)NULL);
        }
    }
}

