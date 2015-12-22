#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <signal.h>
#include <time.h>
#include <string.h>

#define SIGUSR3 SIGWINCH

typedef struct work {
    int type;
    struct itimerval tim;
    struct work *next;
} Work;

Work *head, *curr;

void init_schedule() {
    head = (Work*)malloc(sizeof(Work));
    curr = head;
}

int push_work(int c, int pt_sec, int pt_usec, int lt_sec, int lt_usec) {
    curr->next = (Work*)malloc(sizeof(Work));
    curr = curr->next;
    curr->type = c;
    curr->tim.it_value.tv_sec = pt_sec;
    curr->tim.it_value.tv_usec = pt_usec;
    curr->tim.it_interval.tv_sec = 0;
    curr->tim.it_interval.tv_usec = 0;
    curr->next = NULL;
    return 1;
}

int top_work(int *c, struct itimerval *t) {
    if (head->next == NULL)
        return 0;
    else {
        *c = head->next->type;
        *t = head->next->tim;
    }
    return 1;
}

int pop_work() {
    if (head->next == NULL)
        return 0;
    else {
        Work* tmp = head->next;
        if (tmp == curr)
            curr = head;
        head->next = tmp->next;
        free(tmp);
    }
    return 1;
}

FILE *fp_log;
pid_t childPID;
int serialNum[3];

static void handle_ordinary(int signo) {
    // process time = 0.5, limit time = 2.0
    puts("receive 0");
    fprintf(fp_log, "receive 0 %d\n", ++serialNum[0]);

    kill(childPID, SIGUSR1);
    fprintf(fp_log, "finish 0 %d\n", serialNum[0]);
}

static void handle_patient(int signo) {
    // process time = 1.0, limit time = 3.0
    puts("receive 1");
    fprintf(fp_log, "receive 1 %d\n", ++serialNum[1]);

    kill(childPID, SIGUSR2);
    fprintf(fp_log, "finish 1 %d\n", serialNum[1]);
}

static void handle_impatient(int signo) {
    // process time = 0.2, limit time = 0.3
    puts("receive 2");
    fprintf(fp_log, "receive 2 %d\n", ++serialNum[2]);

    kill(childPID, SIGUSR3);
    fprintf(fp_log, "finish 2 %d\n", serialNum[2]);
}

int mycomp(char buf[]) {
    buf[9] = '\0';
    int res = 0;
    if (strcmp(buf, "terminate") == 0)
        res = 1;
    memset(buf, 0, sizeof(buf));
    return res;
}

int main(int argc, char const *argv[]) {
    
    if (argc < 2) {
        fputs("usage: ./bidding_system_EDF [test_data]", stderr);
        exit(0);
    }

    fprintf(stderr, "my process ID = %d\n", getpid());

    fp_log = fopen("bidding_system_log", "w");
    if (fp_log == NULL) {
        perror("log open error");
        exit(0);
    }

    // init signal handler
    struct sigaction act1, act2, act3;
    act1.sa_handler = handle_ordinary;
    act2.sa_handler = handle_patient;
    act3.sa_handler = handle_impatient;
    act1.sa_flags = 0;
    act2.sa_flags = 0;
    act3.sa_flags = 0;

    // install signal handler
    if (sigaction(SIGUSR1, &act1, NULL) < 0) {
        perror("sigaction");
        exit(1);
    }
    if (sigaction(SIGUSR2, &act2, NULL) < 0) {
        perror("sigaction");
        exit(1);
    }
    if (sigaction(SIGUSR3, &act3, NULL) < 0) {
        perror("sigaction");
        exit(1);
    }

    int pfd_ctob[2], pfd_btoc[2];
    pipe(pfd_ctob);
    pipe(pfd_btoc);
    init_schedule();
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
        execlp("./customer_EDF", "./customer_EDF", argv[1], (char*)NULL);
    }

    close(pfd_ctob[1]);
    close(pfd_btoc[0]);
    int res;
    char buf[32];

    while (1) {
        res = read(pfd_ctob[0], buf, 32);
        if (res < 0 && errno != EINTR)
            continue;
        else if (mycomp(buf))
            break;
        else if (res == 0)
            break;
    }

    fprintf(fp_log, "terminate\n");
    puts("terminate");

    int status;
    waitpid(childPID, &status, 0);

    fclose(fp_log);
    close(pfd_ctob[0]);
    close(pfd_btoc[1]);

    return 0;
}

