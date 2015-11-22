#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
    
    if (argc != 4) {
        fprintf(stderr, "usage: ./player [host_id] [player_index] [random_key]\n");
        exit(-1);
    }

    char i_fifoname[20];
    char o_fifoname[20];
    sprintf(i_fifoname, "host%s_%s.FIFO", argv[1], argv[2]);
    sprintf(o_fifoname, "host%s.FIFO", argv[1]);

    char buf[128], w_buf[128];
    int bid[4];
    memset(buf, 0, sizeof(char) * 128);
    int fifo_htop = open(i_fifoname, O_RDONLY);
    int fifo_ptoh = open(o_fifoname, O_WRONLY | O_APPEND);

    read(fifo_htop, buf, 128);

    sscanf(buf, "%d %d %d %d\n", &bid[0], &bid[1], &bid[2], &bid[3]);
    memset(w_buf, 0, sizeof(char) * 128);
    sprintf(w_buf, "%s %s 1000\n", argv[2], argv[3]);
    write(fifo_ptoh, w_buf, strlen(w_buf));

    close(fifo_htop);
    close(fifo_ptoh);

    return 0;
}

