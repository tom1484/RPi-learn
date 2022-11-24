#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {

    int p_out = atoi(argv[1]);

    char msg[1024] = "Hi father";
    write(p_out, msg, strlen(msg) + 1);
    close(p_out);
    exit(0);

    return 0;
}
