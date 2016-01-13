#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>

static volatile int exit_request = 0;
static volatile int vipIsTimeout, memIsTimeout;
static volatile int vipTimerIsSet, memTimerIsSet;
struct itimerval vip_timer, mem_timer;
FILE *fp_data, *fp_log;
pid_t parentPID;
static volatile int totalCus, accumCus;
int serialNum[3];

typedef struct node {
    int cmd;
    double tim;
    struct node *next;
} Node;

Node *head, *curr;

static void ordinary_handler (int signo) {
    fprintf(fp_log, "finish 0 %d\n", serialNum[0]);
    accumCus++;
}

static void member_handler (int signo) {
    memTimerIsSet = 0;
    memIsTimeout = 0;
    fprintf(fp_log, "finish 1 %d\n", serialNum[1]);
    accumCus++;
}

static void vip_handler (int signo) {
    vipTimerIsSet = 0;
    vipIsTimeout = 0;
    fprintf(fp_log, "finish 2 %d\n", serialNum[2]);
    accumCus++;
}

static void timer_handler (int signo) {
    if (vipTimerIsSet) {
        vipTimerIsSet = 0;
        if (vipIsTimeout) {
            vipIsTimeout = 0;
            fprintf(fp_log, "timeout 2 %d\n", serialNum[2]);
            exit(0);
        }
        if (mem_timer.it_value.tv_usec != 0) {
            setitimer(ITIMER_REAL, &mem_timer, 0);
            memTimerIsSet = 1;
            memIsTimeout = 1;
        }
    }
    else if (memTimerIsSet) {
        memTimerIsSet = 0;
        if (memIsTimeout) {
            memIsTimeout = 0;
            fprintf(fp_log, "timeout 1 %d\n", serialNum[1]);
            exit(0);
        }
        if (vip_timer.it_value.tv_usec != 0) {
            setitimer(ITIMER_REAL, &vip_timer, 0);
            vipTimerIsSet = 1;
            vipIsTimeout = 1;
        }
    }
}

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

int timerComp(struct itimerval a, struct itimerval b) {
    if (a.it_value.tv_sec < b.it_value.tv_sec)
        return 1;
    else if (a.it_value.tv_sec == b.it_value.tv_sec) {
        if (a.it_value.tv_usec < b.it_value.tv_usec)
            return 1;
        else
            return 0;
    }
    else
        return 0;
}

void sendCustomer(int type) {
    struct itimerval t;
    memset(&t, 0, sizeof(t));
    if (type == 0) {
        serialNum[0]++;
        fprintf(fp_log, "send 0 %d\n", serialNum[0]);
        write(STDOUT_FILENO, "ordinary\n", strlen("ordinary\n"));
    }
    else if (type == 1) {
        serialNum[1]++;
        memIsTimeout = 1;
        fprintf(fp_log, "send 1 %d\n", serialNum[1]);
        kill(parentPID, SIGUSR1);
        
        setitimer(ITIMER_REAL, &t, &vip_timer);
        if (vip_timer.it_value.tv_usec != 0) {
            mem_timer.it_value.tv_sec = 0;
            mem_timer.it_value.tv_usec = 1000000 - vip_timer.it_value.tv_usec;
            memTimerIsSet = 0;
            vipTimerIsSet = 1;
            setitimer(ITIMER_REAL, &vip_timer, 0);
        }
        else {
            mem_timer.it_value.tv_sec = 1;
            mem_timer.it_value.tv_usec = 0;
            vipTimerIsSet = 0;
            memTimerIsSet = 1;
            setitimer(ITIMER_REAL, &mem_timer, 0);
        }
    }
    else {
        serialNum[2]++;
        vipIsTimeout = 1;
        fprintf(fp_log, "send 2 %d\n", serialNum[2]);
        kill(parentPID, SIGUSR2);

        vip_timer.it_value.tv_sec = 0;
        vip_timer.it_value.tv_usec = 300000;
        setitimer(ITIMER_REAL, &t, &mem_timer);
        if (mem_timer.it_value.tv_sec != 0 ||
            mem_timer.it_value.tv_usec != 0) {
            if (timerComp(mem_timer, vip_timer)) {
                vip_timer.it_value.tv_usec -= mem_timer.it_value.tv_usec;
                memTimerIsSet = 1;
                vipTimerIsSet = 0;
                setitimer(ITIMER_REAL, &mem_timer, 0);
            }
            else {
                if (mem_timer.it_value.tv_sec > 0) {
                    mem_timer.it_value.tv_sec = 0;
                    mem_timer.it_value.tv_usec = 1000000;
                }
                mem_timer.it_value.tv_usec -= vip_timer.it_value.tv_usec;
                memTimerIsSet = 0;
                vipTimerIsSet = 1;
                setitimer(ITIMER_REAL, &vip_timer, 0);
            }
        }
        else {
            memTimerIsSet = 0;
            vipTimerIsSet = 1;
            setitimer(ITIMER_REAL, &vip_timer, 0);
        }
    }
}

int main (int argc, char const *argv[]) {

    if (argc < 2) {
        fputs("usage: ./customer [test_data]", stderr);
        exit(1);
    }
    
    fp_data = fopen(argv[1], "r");
    fp_log = fopen("customer_log", "w");
    if (fp_data == NULL || fp_log == NULL) {
        perror("fopen");
        exit(1);
    }

    int total = 0;
    char buf[128];
    int data_cus[256];
    double data_tim[256];
    data_cus[0] = 0;
    data_tim[0] = 0.0;
    while (fgets(buf, 128, fp_data)) {
        total++;
        sscanf(buf, "%d %lf", &data_cus[total], &data_tim[total]);
        totalCus++;
    }

    struct sigaction act1, act2, act3, act4;
    memset (&act1, 0, sizeof (act1));
    act1.sa_handler = ordinary_handler;
    act1.sa_flags = 0;
    sigaction (SIGINT, &act1, NULL);

    memset (&act2, 0, sizeof (act2));
    act2.sa_handler = member_handler;
    act2.sa_flags = 0;
    sigaction (SIGUSR1, &act2, NULL);

    memset (&act3, 0, sizeof (act3));
    act3.sa_handler = vip_handler;
    act3.sa_flags = 0;
    sigaction (SIGUSR2, &act3, NULL);

    memset(&act4, 0, sizeof(act4));
    act4.sa_handler = timer_handler;
    act4.sa_flags = 0;
    sigemptyset(&act4.sa_mask);
    sigaddset(&act4.sa_mask, SIGUSR1);
    sigaddset(&act4.sa_mask, SIGUSR2);
    sigaddset(&act4.sa_mask, SIGINT);
    sigaction (SIGALRM, &act4, NULL);

    /* Do busy work. */
    memset(serialNum, 0, sizeof(int) * 3);
    memset(&vip_timer, 0, sizeof(vip_timer));
    memset(&mem_timer, 0, sizeof(mem_timer));
    parentPID = getppid();
    int index = 1;

    while (accumCus != totalCus) {
        if (index <= total) {
            msleep(data_tim[index] - data_tim[index-1]);
            sendCustomer(data_cus[index++]);
        }
    }

    fclose(fp_data);
    fclose(fp_log);

    return 0;
}

