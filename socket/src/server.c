#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
    int socket_fd = socket(PF_INET, SOCK_STREAM, 0);

    if (socket_fd == -1) {
        perror("can not create socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    size_t addr_len = sizeof(struct sockaddr_in);
    memset(&server_addr, 0, addr_len);

    server_addr.sin_family = PF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(socket_fd, (const struct sockaddr *)&server_addr, addr_len)) {
        perror("error bind failed");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(socket_fd, 10) == -1) {
        perror("error listen failed");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    while (1) {
        int connect_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &client_len);

        if (connect_fd < 0) {
            perror("error accept failed");
            close(socket_fd);
            exit(EXIT_FAILURE);
        }
        printf("accept connection from client\n");

        char msg[1024] = "accepted";
        if (send(connect_fd, msg, sizeof(msg), 0) < 0) {
            perror("error write failed");
            close(socket_fd);
            exit(EXIT_FAILURE);
        }

        char buf[1024];
        if (recv(connect_fd, buf, sizeof(buf), 0) < 0) {
            perror("error recv failed");
            close(socket_fd);
            exit(EXIT_FAILURE);
        }
        printf("received: %s\n", buf);

        shutdown(connect_fd, SHUT_RDWR);
        close(connect_fd);
    }

    return 0;
}
