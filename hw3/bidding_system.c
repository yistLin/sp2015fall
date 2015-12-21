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

static void handle_ordinary() {
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 50000000L;

    puts("receive 0");
    nanosleep(&t, NULL);
    kill(childPID, SIGINT);
}

static void handle_member(int signo) {
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 50000000L;

    puts("receive 1");
    nanosleep(&t, NULL);
    kill(childPID, SIGUSR1);
}

static void handle_vip(int signo) {
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 50000000L;
    
    puts("receive 2");
    nanosleep(&t, NULL);
    kill(childPID, SIGUSR2);
}

int mycomp(char buf[]) {
    buf[8] = '\0';
    if (strcmp(buf, "ordinary") == 0) {
        memset(buf, 0, sizeof(buf));
        return 1;
    }
    else {
        memset(buf, 0, sizeof(buf));
        return 0;
    }
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

    int pfd_ctob[2];
    pipe(pfd_ctob);
    childPID = fork();
    if (childPID < 0) {
        perror("fork error");
        exit(0);
    }
    else if (childPID == 0) {
        // fork a child process and execute ./customer
        dup2(pfd_ctob[1], STDOUT_FILENO);
        close(pfd_ctob[1]);
        execlp("./customer", "./customer", argv[1], (char*)NULL);
    }

    close(pfd_ctob[1]);
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
    int status;
    waitpid(childPID, &status, 0);
    fclose(fp_log);
    close(pfd_ctob[0]);

    return 0;
}

