#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>

static volatile int cntOrd = 0, cntMem = 0, cntVip = 0;
FILE *fp_data, *fp_log;

typedef struct node {
    int cmd;
    double tim;
    struct node *next;
} Node;

Node *head, *curr;

static void ordinary_handler (int signo) {
    --cntOrd;
}

static void member_handler (int signo) {
    --cntMem;
}

static void vip_handler (int signo) {
    --cntVip;
}

void init_list() {
    head = (Node*)malloc(sizeof(Node));
    curr = head;
}

int push_list(int c,  double t) {
    curr->next = (Node*)malloc(sizeof(Node));
    curr = curr->next;
    curr->cmd = c;
    curr->tim = t;
    curr->next = NULL;
    return 1;
}

int pop_list(int *c, double *t) {
    if (head->next == NULL)
        return 0;
    else {
        Node* tmp = head->next;
        if (tmp == curr)
            curr = head;
        *c = tmp->cmd;
        *t = tmp->tim;
        head->next = tmp->next;
        free(tmp);
    }
    return 1;
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
    while (fgets(buf, 128, fp_data)) {
        sscanf(buf, "%d %lf", &data_cus[total], &data_tim[total]);
        total++;
    }

    struct sigaction act1, act2, act3;
    memset (&act1, 0, sizeof (act1));
    act1.sa_handler = ordinary_handler;
    sigaction (SIGINT, &act1, NULL);
    memset (&act2, 0, sizeof (act2));
    act2.sa_handler = member_handler;
    sigaction (SIGUSR1, &act2, NULL);
    memset (&act3, 0, sizeof (act3));
    act3.sa_handler = vip_handler;
    sigaction (SIGUSR2, &act3, NULL);

    /* Do busy work. */
    struct itimerval timer;
    pid_t parentPID = getppid();
    while (1) {
        
        timer.it_value.tv_sec = 0;
        timer.it_value.tv_usec = 250000;
        timer.it_interval.tv_sec = 0;
        timer.it_interval.tv_usec = 0;
        setitimer (ITIMER_VIRTUAL, &timer, NULL);
    }

    fclose(fp_data);
    fclose(fp_log);

    return 0;
}

