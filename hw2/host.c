#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

    if (argc != 2) {
        fputs("usage: ./host [host_id]", stderr);
        exit(0);
    }

    int host_id = atoi(argv[1]);

    char buf[1024];
    read(0, buf, 1024);

    fprintf(stderr, "HOST: id = %d. I read \"%s\" from STDIN\n", host_id, buf);

    return 0;
}

