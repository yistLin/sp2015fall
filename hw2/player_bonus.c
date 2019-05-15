/*b03902048 林義聖*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>

int cmpfunc(const void* a, const void* b) {
    return *(int*)b - *(int*)a;
}

int main(int argc, char *argv[]) {
    
    if (argc != 4) {
        fprintf(stderr, "usage: ./player [host_id] [player_index] [random_key]\n");
        exit(-1);
    }

    srand((unsigned int) time(NULL));

    char i_fifoname[20];
    char o_fifoname[20];
    sprintf(i_fifoname, "host%s_%s.FIFO", argv[1], argv[2]);
    sprintf(o_fifoname, "host%s.FIFO", argv[1]);

    char index = argv[2][0];
    char buf[32], w_buf[32];
    int bid[4];
    int myturn = index - 'A';
    int fifo_htop = open(i_fifoname, O_RDONLY);
    int fifo_ptoh = open(o_fifoname, O_WRONLY | O_APPEND);

    int mybid, randnum;

    for (int i = 0; i < 10; i++) {

        memset(buf, 0, sizeof(char) * 32);
        read(fifo_htop, buf, 32);
        sscanf(buf, "%d %d %d %d\n", &bid[0], &bid[1], &bid[2], &bid[3]);
        
        mybid = bid[myturn];
        for (int j = 0; j < (2 * myturn + 1); j++)
            randnum = (int)rand() % 30 + 60;

        qsort(bid, 4, sizeof(int), cmpfunc);

        if (mybid <= bid[0])
            mybid = mybid * randnum / 100;
        else
            mybid = bid[0] + 1;

        memset(w_buf, 0, sizeof(char) * 32);
        sprintf(w_buf, "%s %s %d\n", argv[2], argv[3], mybid);
        write(fifo_ptoh, w_buf, strlen(w_buf));
    }

    close(fifo_htop);
    close(fifo_ptoh);

    return 0;
}

