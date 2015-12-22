#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/select.h>
#include <signal.h>
#include <time.h>
#include <string.h>

FILE *fp_log;
pid_t childPID;
int serialNum[3];

int msleep(double sleept) {
    struct timespec t;
    t.tv_sec = (time_t) sleept;
    t.tv_nsec = (long long int)((sleept - t.tv_sec) * 1e+9);

    while (1) {
        int rval = nanosleep(&t, &t);
        if (rval == 0) // all done
            return 0;
        else if (errno == EINTR) // interrupted by signal
            continue;
        else // some error
            return rval;
    }
    return 0;
}

static void handle_ordinary() {
    serialNum[0]++;
    puts("receive 0");
    fprintf(fp_log, "receive 0 %d\n", serialNum[0]);

    msleep(1.0);

    kill(childPID, SIGINT);
    fprintf(fp_log, "finish 0 %d\n", serialNum[0]);
}

static void handle_member(int signo) {
    serialNum[1]++;
    puts("receive 1");
    fprintf(fp_log, "receive 1 %d\n", serialNum[1]);

    msleep(0.5);

    kill(childPID, SIGUSR1);
    fprintf(fp_log, "finish 1 %d\n", serialNum[1]);
}

static void handle_vip(int signo) {
    serialNum[2]++;
    puts("receive 2");
    fprintf(fp_log, "receive 2 %d\n", serialNum[2]);

    msleep(0.2);

    kill(childPID, SIGUSR2);
    fprintf(fp_log, "finish 2 %d\n", serialNum[2]);
}

int mycomp(char buf[]) {
    buf[8] = '\0';
    int res = 0;
    if (strcmp(buf, "ordinary") == 0)
        res = 1;
    memset(buf, 0, sizeof(buf));
    return res;
}

int main(int argc, char const *argv[]) {
    
    if (argc < 2) {
        fputs("usage: ./bidding_system [test_data]", stderr);
        exit(0);
    }

    fprintf(stderr, "my process ID = %d\n", getpid());

    fp_log = fopen("bidding_system_log", "w");
    if (fp_log == NULL) {
        perror("log open error");
        exit(0);
    }

    // init signal handler
    struct sigaction act1, act2;
    act1.sa_handler = handle_member;
    act2.sa_handler = handle_vip;
    act1.sa_flags = 0;
    act2.sa_flags = 0;
    sigemptyset(&act1.sa_mask);
    sigemptyset(&act2.sa_mask);
    sigaddset(&act2.sa_mask, SIGUSR1);
    sigaddset(&act2.sa_mask, SIGUSR2);


    // install signal handler
    if (sigaction(SIGUSR1, &act1, NULL) < 0) {
        perror("sigaction");
        exit(1);
    }
    if (sigaction(SIGUSR2, &act2, NULL) < 0) {
        perror("sigaction");
        exit(1);
    }

    int pfd_ctob[2], pfd_btoc[2];
    pipe(pfd_ctob);
    pipe(pfd_btoc);
    memset(serialNum, 0, 3 * sizeof(int));

    childPID = fork();
    if (childPID < 0) {
        perror("fork error");
        exit(0);
    }
    else if (childPID == 0) {
        // fork a child process and execute ./customer
        dup2(pfd_ctob[1], STDOUT_FILENO);
        dup2(pfd_btoc[0], STDIN_FILENO);
        close(pfd_ctob[1]);
        close(pfd_btoc[0]);
        execlp("./customer", "./customer", argv[1], (char*)NULL);
    }

    close(pfd_ctob[1]);
    close(pfd_btoc[0]);
    int res;
    char buf[32];

    while (1) {
        res = read(pfd_ctob[0], buf, 32);
        if (res < 0 && errno != EINTR)
            continue;
        else if (res == 0)
            break;
        else if (mycomp(buf))
            handle_ordinary();
    }

    puts("terminate");
    fprintf(fp_log, "terminate\n");

    int status;
    waitpid(childPID, &status, 0);

    fclose(fp_log);
    close(pfd_ctob[0]);
    close(pfd_btoc[1]);

    return 0;
}

