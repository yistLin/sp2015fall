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
    tim.tv_sec = 2;
    tim.tv_nsec = 500000000L;

    pid_t ppid = getppid();
    int i;
    char buf[128];
    for (i=0; i<10; i++) {
        nanosleep(&tim, NULL);
        if (i % 3 == 0) {
            sprintf(buf, "%d: kerker\n", i+1);
            write(1, buf, strlen(buf));
        }
        else if (i % 3 == 1) {
            kill(ppid, SIGUSR1);
        }
        else {
            kill(ppid, SIGUSR2);
        }
    }
    kill(ppid, SIGTERM);
    return 0;
}
