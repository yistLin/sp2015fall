#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>

int main(int argc, char const *argv[])
{
    struct timespec tim;
    tim.tv_sec = 0;
    tim.tv_nsec = 500000000L;

    pid_t ppid = getppid();
    int i;
    for (i=0; i<10; i++) {
        nanosleep(&tim, NULL);
        if (i % 3 == 0) {
            write(1, "ordinary\n", strlen("ordinary\n"));
        }
        else if (i % 3 == 1) {
            kill(ppid, SIGUSR1);
        }
        else {
            kill(ppid, SIGUSR2);
        }
    }
    return 0;
}
