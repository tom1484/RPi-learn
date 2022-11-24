#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>

const int CHILD_NUM = 2;
int child_status[2];

void on_sigchld(int sig_num) {
    pid_t pid;
    int status;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("%d died\n", pid);
        for (int i = 0; i < CHILD_NUM; i++) {
            if (pid == child_status[i]) {
                child_status[i] = 0;
            }
        }
    }
}

void child_routime() {
    printf("%d started\n", getpid());
    while (1) {}
}

int main() {
    pid_t pid;
    signal(SIGCHLD, on_sigchld);

    for (int i = 0; i < CHILD_NUM; i++) {
        child_status[i] = 0;
    }

    while (1) {
        for (int i = 0; i < CHILD_NUM; i++) {
            if (child_status[i] == 0) {
                if ((child_status[i] = fork()) == 0) {
                    child_routime();
                }
            }
        }
    }

    return 0;
}
