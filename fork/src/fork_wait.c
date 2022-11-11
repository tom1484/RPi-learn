#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <stdlib.h>

int main() {
    pid_t pid = fork();
    int status;

    if (pid == 0) {
        printf("Child %d\n", getpid());
        exit(0);
    }
    else if (pid > 0) {
        printf("Parent %d\n", getpid());
        wait(&status);
        printf("Received %d\n", status);
        sleep(100);
    }
    else {
        printf("Unknown %d\n", pid);
    }

    return 0;
}
