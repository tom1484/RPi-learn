#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

int main(int argc, char *argv[]) {

    int server_fd;
    if ((server_fd = socket(PF_INET, SOCK_STREAM, 0)) == 0) {
        perror("can not create socket");
        exit(EXIT_FAILURE);
    }
    printf("socket opened\n");

    int sock_opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&sock_opt, sizeof(sock_opt)) < 0) {
        perror("set socket option failed");
        exit(EXIT_FAILURE);
    }

    int max_clients = 10;
    int client_fds[max_clients];
    for (int i = 0; i < max_clients; i++) {
        client_fds[i] = 0;
    }

    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    memset(&server_addr, 0, addr_len);

    int port = atoi(argv[1]);
    server_addr.sin_family = PF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (const struct sockaddr *)&server_addr, addr_len)) {
        perror("error bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) == -1) {
        perror("error listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    fd_set read_fds;
    while (1) {
        FD_ZERO(&read_fds);

        int max_sd;
        FD_SET(server_fd, &read_fds);
        max_sd = server_fd;

        int sd;
        for (int i = 0; i < max_clients; i++) {
            sd = client_fds[i];
            if (sd > 0) {
                FD_SET(sd, &read_fds);
                if (sd > max_sd) {
                    max_sd = sd;
                }
            }
        }

        int activity;
        if ((activity = select(max_sd + 1, &read_fds, NULL, NULL, NULL)) < 0 && errno != EINTR) {
            printf("select error\n");
        }

        int new_conn_fd;
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        if (FD_ISSET(server_fd, &read_fds)) {
            if ((new_conn_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len)) < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            printf("new connection from %s:%d\n", 
                    inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            for (int i = 0; i < max_clients; i++) {
                if (client_fds[i] == 0) {
                    client_fds[i] = new_conn_fd;
                    break;
                }
            }
        }

        char buf[1024];
        char msg[1024];
        int read_len;
        for (int i = 0; i < max_clients; i++) {
            sd = client_fds[i];

            if (FD_ISSET(sd, &read_fds)) {
                getpeername(sd, (struct sockaddr *)&client_addr, &client_len);
                if ((read_len = read(sd, buf, 1024)) == 0) {
                    printf("host %s:%d disconnectd\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                    close(sd);
                    client_fds[i] = 0;
                }
                else {
                    printf("host %s:%d said:\n\t", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                    buf[read_len] = '\0';
                    printf("%s\n", buf);
                    sprintf(msg, "received %d bytes", read_len);
                    send(sd, msg, strlen(msg), 0);
                }
            }
        }

        /* int connect_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len); */

        /* if (connect_fd < 0) { */
        /*     perror("error accept failed"); */
        /*     close(server_fd); */
        /*     exit(EXIT_FAILURE); */
        /* } */
        /* printf("accept connection from client\n"); */

        /* char msg[1024] = "accepted"; */
        /* if (send(connect_fd, msg, sizeof(msg), 0) < 0) { */
        /*     perror("error write failed"); */
        /*     close(server_fd); */
        /*     exit(EXIT_FAILURE); */
        /* } */

        /* char buf[1024]; */
        /* if (recv(connect_fd, buf, sizeof(buf), 0) < 0) { */
        /*     perror("error recv failed"); */
        /*     close(server_fd); */
        /*     exit(EXIT_FAILURE); */
        /* } */
        /* printf("received: %s\n", buf); */

        /* shutdown(connect_fd, SHUT_RDWR); */
        /* close(connect_fd); */
    }

    return 0;
}
