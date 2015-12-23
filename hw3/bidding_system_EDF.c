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
    int type, firstdeal;
    struct timespec tim, lim;
    struct work *next;
} Work;

Work *head, *curr;
FILE *fp_log;
pid_t childPID;
int serialNum[3];
int signal_arr[3];

void init_schedule() {
    head = (Work*)malloc(sizeof(Work));
    curr = head;
}

int run_work() {
    Work *first = head->next;
    Work *tmp = head->next;
    struct timespec t;
    if (first != NULL) {
        if (first->firstdeal == 1) {
            fprintf(fp_log, "receive %d %d\n", first->type, ++serialNum[first->type]);
            first->firstdeal = 0;
        }
        t.tv_sec = first->tim.tv_sec;
        t.tv_nsec = first->tim.tv_nsec;
        while (tmp != NULL) {
            tmp->lim.tv_sec -= t.tv_sec;
            if (tmp->lim.tv_nsec < t.tv_nsec) {
                (tmp->lim.tv_sec)--;
                (tmp->lim.tv_nsec) += 1e+9;
            }
            tmp->lim.tv_nsec -= t.tv_nsec;
            tmp = tmp->next;
        }
        if (nanosleep(&(first->tim), &(first->tim)) == 0) {
            // the work is done
            kill(childPID, signal_arr[first->type]);
            fprintf(fp_log, "finish %d %d\n", first->type, serialNum[first->type]);
            head->next = first->next;
            free(first);
        }
        return run_work();
    }
    return 1;
}

int compLimit(struct timespec a, struct timespec b) {
    if (a.tv_sec < b.tv_sec)
        return 1;
    else if (a.tv_sec == b.tv_sec) {
        if (a.tv_nsec < b.tv_sec)
            return 1;
        else
            return 0;
    }
    else
        return 0;
}

int insert_work(int c, double pt, double lt) {
    // get remaining time
    // puts("In insert_work()");
    struct timespec rem;
    memset(&rem, 0, sizeof(rem));
    Work *tmp = head->next;
    if (tmp != NULL) {
        rem.tv_sec = tmp->tim.tv_sec;
        rem.tv_nsec = tmp->tim.tv_nsec;
    }

    // add remaining time back to limit time
    while (tmp != NULL) {
        (tmp->lim.tv_sec) += rem.tv_sec;
        (tmp->lim.tv_nsec) += rem.tv_nsec;
        if ((tmp->lim.tv_nsec) > 1e+9) {
            tmp->lim.tv_nsec -= 1e+9;
            (tmp->lim.tv_sec)++;
        }
        tmp = tmp->next;
    }

    // create new work
    Work *nwk = (Work*)malloc(sizeof(Work));
    nwk->type = c;
    nwk->firstdeal = 1;
    nwk->tim.tv_sec = (time_t)pt;
    nwk->tim.tv_nsec = (long long int)((pt - (time_t)pt) * 1e+9);
    nwk->lim.tv_sec = (time_t)lt;
    nwk->lim.tv_nsec = (long long int)((lt - (time_t)lt) * 1e+9);

    // insert new work
    tmp = head->next;
    Work *prev = head;
    while (tmp != NULL && compLimit(tmp->lim, nwk->lim)) {
        prev = tmp;
        tmp = tmp->next;
    }
    prev->next = nwk;
    nwk->next = tmp;
    run_work();
    return 1;
}

static void handle_ordinary(int signo) {
    // process time = 0.5, limit time = 2.0
    insert_work(0, 0.5, 2.0);
}

static void handle_patient(int signo) {
    // process time = 1.0, limit time = 3.0
    insert_work(1, 1.0, 3.0);
}

static void handle_impatient(int signo) {
    // process time = 0.2, limit time = 0.3
    insert_work(2, 0.2, 0.3);
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

    // fprintf(stderr, "my process ID = %d\n", getpid());

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
    sigemptyset(&act1.sa_mask);
    sigemptyset(&act2.sa_mask);
    sigemptyset(&act3.sa_mask);
    signal_arr[0] = SIGUSR1;
    signal_arr[1] = SIGUSR2;
    signal_arr[2] = SIGUSR3;

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

    int status;
    waitpid(childPID, &status, 0);

    fclose(fp_log);
    close(pfd_ctob[0]);
    close(pfd_btoc[1]);

    return 0;
}

