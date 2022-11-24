#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <stdlib.h>

int main() {

    int argc;
    char *argv[10];

    int comm_len;
    char command[1024];

    while (1) {
        argc = 0;
        // Read command from stdin.
        fgets(command, (int)sizeof(command), stdin);

        // Tidious string separation.
        char *sp_ptr = command;
        while (1) {
            int n_cpy = 0;
            while (sp_ptr[n_cpy] != ' ' && sp_ptr[n_cpy] != '\n')
                n_cpy++;

            argv[argc] = malloc(n_cpy);
            strncpy(argv[argc], sp_ptr, n_cpy);
            argc++;

            sp_ptr += n_cpy;
            if (*sp_ptr == '\n')
                break;
            sp_ptr++;
        }
        argv[argc] = NULL;

        // Fork a child process.
        pid_t pid = fork();
        if (pid == 0) {
            // If it's a child, execute the given command.
            /* printf("%s", argv[0]); */
            execvp(argv[0], argv);
        }

        // Wait for child ending.
        int stat;
        waitpid(pid, &stat, 0);
        printf("execution result: %d\n", stat);

        if (stat) {
            break;
        }
    }


    return 0;
}
