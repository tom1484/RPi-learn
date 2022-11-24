#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>


int main() {

    int p[2];

    if (pipe(p) < 0) {
        perror("pipe create error");
        exit(EXIT_FAILURE);
    }

    int pid = fork();
    if (pid == 0) {
        // Child code
        close(p[0]);

        char p_str[10];
        sprintf(p_str, "%d", p[1]);
        execl("./bin/bin", "bin", p_str, NULL);
    }
    // Parent code
    close(p[1]);

    char buf[1024];
    read(p[0], buf, 1024);
    printf("%s\n", buf);
    close(p[0]);

    int stat;
    waitpid(pid, &stat, 0);
    printf("child end with %d\n", stat);

    return 0;
}
