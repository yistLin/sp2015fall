#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

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

    int **pipefd, *pData;
    pipefd = (int**)malloc(host_num * sizeof(int*));
    pData = (int*)malloc(2 * host_num * sizeof(int));
    for (int i = 0; i < host_num; i++, pData += 2) {
        pipefd[i] = pData;
        pipe(pipefd[i]);
    }

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
            dup2(pipefd[i][0], 0);
            for (int j = 0; j < host_num; j++) {
                close(pipefd[j][0]);
                close(pipefd[j][1]);
            }

            sprintf(hostid, "%d", i+1);
            execlp("./host", "hw2/host", hostid, (char*)NULL);
            break;
        }
    }

    fprintf(stderr, "SYSTEM: pid = %d\n", getpid());

    char *words_for_child = "hehehe";
    for (int i = 0; i < host_num; i++) {
        write(pipefd[i][1], words_for_child, strlen(words_for_child));
    }

    free(pipefd);

    return 0;
}

