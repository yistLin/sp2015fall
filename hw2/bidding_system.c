#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/select.h>
#include <sys/types.h>

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

    fprintf(stderr, "SYSTEM: pid = %d\n", getpid());

    for (int i = 0; i < host_num; i++) {
        close(pfd_htos[i][1]);
        close(pfd_stoh[i][0]);
    }

    fd_set readset, allset;
    FD_ZERO(&allset);
    int max_fd = 0;
    srand(time(NULL));

    char end_competition[1024];
    sprintf(end_competition, "-1 -1 -1 -1\n");
    int len = strlen(end_competition);
    for (int i = 0; i < host_num; i++) {
        write(pfd_stoh[i][1], end_competition, len);
        FD_SET(pfd_htos[i][0], &allset);
        if (pfd_htos[i][0] > max_fd)
            max_fd = pfd_htos[i][0];
    }
/*
    int counter = host_num;
    char buf[1024];
    memset(buf, 0, sizeof(char) * 1024);
    while (1) {
        readset = allset;
        select(max_fd + 1, &readset, NULL, NULL, NULL);

        for (int i = 0; i < host_num; i++) {
            if (FD_ISSET(pfd_htos[i][0], &readset)) {
                read(pfd_htos[i][0], buf, 1024);
                fprintf(stderr, "SYSTEM: %s\n", buf);
                FD_CLR(pfd_htos[i][0], &allset);
                counter--;
            }
        }

        if (counter <= 0)
            break;
    }
*/
    free(pfd_stoh);
    free(pfd_htos);

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
