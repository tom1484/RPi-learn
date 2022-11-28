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

int main(int argc, char *argv[])
{

    // Create socket.
    int server_fd;
    if ((server_fd = socket(PF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("can not create socket");
        exit(EXIT_FAILURE);
    }
    printf("socket opened\n");

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
    if (bind(server_fd, (const struct sockaddr *)&server_addr, addr_len))
    {
        perror("error bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Start listen to socket with maximum queue size N.
    if (listen(server_fd, 10) == -1)
    {
        perror("error listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    char command[32];
    int com_num;
    /* fd_set fdset; */
    char buf[1024];
    int read_len;
    char msg[1024];
    int send_len;

    int client_fd = 0;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    while (1)
    {
        // If no client is set, try to connect to new one.
        // Otherwise just ignore.
        if (client_fd == 0)
        {
            if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len)) < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            printf("new connection from %s:%d\n",
                   inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        }
        // If connection exists, read command from user and send.
        else
        {
            printf("command: ");

            scanf("%s", command);
            if (strcmp(command, "q") == 0)
            {
                printf("quit\n");

                close(client_fd);
                client_fd = 0;
                continue;
            }
            scanf("%d", &com_num);
            sprintf(msg, "%s %d", command, com_num);

            send(client_fd, msg, strlen(msg), 0);

            // Exit if I/O failed.
            if ((read_len = recv(client_fd, buf, 1024, 0)) < 0)
            {
                perror("error read failed");
                close(client_fd);
                exit(EXIT_FAILURE);
            }
            // Disconnect if received nothing.
            else if (read_len == 0)
            {
                getpeername(client_fd, (struct sockaddr *)&client_addr, &client_len);
                printf("host %s:%d disconnectd\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                close(client_fd);
                client_fd = 0;
            }
            else
            {
                buf[read_len] = '\0';
                printf("reply: %s\n", buf);
            }
        }
    }

    return 0;
}
