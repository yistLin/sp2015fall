#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/select.h>
#include <signal.h>

pid_t childPID;

static void handle_ordinary(int n, char *buf) {
    write(1, buf, n);
    kill(childPID, SIGINT);
}

static void handle_member(int signo) {
    puts("In member customer handler");
    kill(childPID, SIGUSR1);
}

static void handle_vip(int signo) {
    puts("In VIP customer handler");
    kill(childPID, SIGUSR2);
}

int main(int argc, char const *argv[]) {
    
    if (argc < 2) {
        fputs("usage: ./bidding_system [test_data]", stderr);
        exit(0);
    }

    fprintf(stderr, "my process ID = %d\n", getpid());

    FILE *fp_log = fopen("bidding_system_log", "w");
    if (fp_log == NULL) {
        perror("log open error");
        exit(0);
    }

    // init signal handler
    sigset_t mask, orig_mask;
    struct sigaction act1, act2;
    act1.sa_handler = handle_member;
    act2.sa_handler = handle_vip;
    sigemptyset(&act2.sa_mask);
    sigaddset(&act2.sa_mask, SIGUSR1);

    // install signal handler
    if (sigaction(SIGUSR1, &act1, NULL) < 0) {
        perror("sigaction");
        exit(1);
    }
    if (sigaction(SIGUSR2, &act2, NULL) < 0) {
        perror("sigaction");
        exit(1);
    }

    // prepare for sigprocmask
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);
    if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0) {
        perror("sigprocmask");
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
    char buf[1024];

    if (sigprocmask(SIG_UNBLOCK, &mask, NULL) < 0) {
        perror("sigprocmask");
        exit(1);
    }

    while (1) {
        res = read(pfd_ctob[0], buf, 1024);
        if (res < 0 && errno != EINTR)
            continue;
        else if (res == 0)
            break;
        else
            handle_ordinary(res, buf);
    }

    int status;
    waitpid(childPID, &status, 0);
    fclose(fp_log);

    return 0;
}