#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/select.h>
#include <signal.h>

static volatile int exit_request = 0;

static void handle_term(int signo) {
    exit_request = 1;
}

static void handle_member(int signo) {
    puts("In member customer handler");
}

static void handle_vip(int signo) {
    puts("In VIP customer handler");
}

static void handle_ordinary(int fd) {
    puts("In ordinary customer handler");
    char buf[1024];
    int n = read(fd, buf, 1024);
    write(1, buf, n);
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
    struct sigaction act1, act2, act;
    act1.sa_handler = handle_member;
    act2.sa_handler = handle_vip;
    act.sa_handler = handle_term;

    // install signal handler
    if (sigaction(SIGUSR1, &act1, 0)) {
        perror("sigaction");
        exit(1);
    }
    if (sigaction(SIGUSR2, &act2, 0)) {
        perror("sigaction");
        exit(1);
    }
    if (sigaction(SIGTERM, &act, 0)) {
        perror("sigaction");
        exit(1);
    }

    // prepare for sigprocmask
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);
    sigaddset(&mask, SIGTERM);
    if (sigprocmask(SIG_BLOCK, &mask, &orig_mask) < 0) {
        perror("sigprocmask");
        exit(1);
    }

    int pfd_ctob[2];
    pipe(pfd_ctob);
    pid_t childPID = fork();
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

    // prepare for pselect
    fd_set fds;
    int res;
    char buf[1024];

    while (!exit_request) {
        FD_ZERO(&fds);
        FD_SET(pfd_ctob[0], &fds);
        res = pselect(pfd_ctob[0] + 1, &fds, NULL, NULL, NULL, &orig_mask);

        if (res < 0 && errno != EINTR) {
            perror("pselect");
            exit(1);
        }
        else if (exit_request) {
            puts("exit");
            exit(0);
        }
        else if (res = 0)
            continue;

        if (FD_ISSET(pfd_ctob[0], &fds)) {
            handle_ordinary(pfd_ctob[0]);
        }
    }

    int status;
    waitpid(childPID, &status, 0);
    fclose(fp_log);

    return 0;
}

