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

    // Create socket.
    int server_fd;
    if ((server_fd = socket(PF_INET, SOCK_STREAM, 0)) == 0) {
        perror("can not create socket");
        exit(EXIT_FAILURE);
    }
    printf("socket opened\n");

    // Allow multiple clients.
    int sock_opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&sock_opt, sizeof(sock_opt)) < 0) {
        perror("set socket option failed");
        exit(EXIT_FAILURE);
    }

    // Clear slots for client file descriptors.
    int max_clients = 10;
    int client_fds[max_clients];
    for (int i = 0; i < max_clients; i++) {
        // 0 represents a empty slot.
        client_fds[i] = 0;
    }

    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    memset(&server_addr, 0, addr_len);

    // Configure server socket info.
    int port = atoi(argv[1]);
    server_addr.sin_family = PF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket address and file descriptor so that incoming
    // connections to the address are associated with the descriptor.
    if (bind(server_fd, (const struct sockaddr *)&server_addr, addr_len)) {
        perror("error bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // start listen to socket with maximum queue size N
    if (listen(server_fd, 10) == -1) {
        perror("error listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Keep executing the following steps to handle connection events.
    // STEP 1. Add all available descriptors into set.
    // STEP 2. Use select() to check the readability of each descriptor.
    // STEP 3. Check if server descriptor if readable.
    //         If it is, then accept the connection.
    // STEP 4. Check if each client descriptor if readable.
    //         If it is, handle disconnection or I/O.
    
    fd_set read_fds;
    while (1) {
        // Clear descriptor set.
        FD_ZERO(&read_fds);

        // Add server descriptor.
        int max_sd;
        FD_SET(server_fd, &read_fds);
        max_sd = server_fd;

        // Add descriptors of all alive clients into set.
        int sd;
        for (int i = 0; i < max_clients; i++) {
            sd = client_fds[i];
            if (sd > 0) {
                FD_SET(sd, &read_fds);
                if (sd > max_sd) {
                    // Maintain the max descriptor number that
                    // needs to be checked.
                    max_sd = sd;
                }
            }
        }

        // Check readability of each descriptors.
        // The corresponding bit in set will be set to 0
        // if a descriptor is not readable.
        int activity;
        if ((activity = select(max_sd + 1, &read_fds, NULL, NULL, NULL)) < 0 && errno != EINTR) {
            printf("select error\n");
        }

        // Accept connection if server descriptor is readable.
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
                    // Insert new descriptor into slots.
                    client_fds[i] = new_conn_fd;
                    break;
                }
            }
        }

        // For readable descriptors, receive incoming message
        // and send response.
        char buf[1024];
        char msg[1024];
        int read_len;
        for (int i = 0; i < max_clients; i++) {
            sd = client_fds[i];

            if (FD_ISSET(sd, &read_fds)) {
                // Get client info of host.
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
    }

    return 0;
}
