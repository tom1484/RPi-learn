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
    if (argc < 3) {
        printf("invalid arguments [<ip> <port>]\n");
        exit(EXIT_FAILURE);
    }

    int socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (socket_fd == -1) {
        perror("cannot create socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in st_sockaddr;
    size_t addr_len = sizeof(struct sockaddr_in);
    memset(&st_sockaddr, 0, sizeof(struct sockaddr_in));

    char *ip_address = argv[1];
    st_sockaddr.sin_family = PF_INET;
    st_sockaddr.sin_port = htons(atoi(argv[2]));
    int res = inet_pton(PF_INET, ip_address, &st_sockaddr.sin_addr);

    if (res < 0) {
        perror("error: first parameter is not a valid address family");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }
    else if (0 == res) {
        perror("char string (second parameter does not contain valid ipaddress");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    if (connect(socket_fd, (struct sockaddr *)&st_sockaddr, addr_len) < 0) {
        perror("error connect failed");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    printf("connected\n");

    char msg[1024];
    char buf[1024];
    size_t read_len;
    while (1) {
        printf("send message [q to quit]:\n\t");
        scanf("%s", msg);

        if (strcmp(msg, "q") == 0) {
            printf("quit\n");
            break;
        }
        else {
            if (send(socket_fd, msg, strlen(msg), 0) < 0) {
                perror("error send failed");
                close(socket_fd);
                exit(EXIT_FAILURE);
            }
            if ((read_len = recv(socket_fd, buf, sizeof(buf), 0)) < 0) {
                perror("error read failed");
                close(socket_fd);
                exit(EXIT_FAILURE);
            }
            buf[read_len] = '\0';
            printf("received response:\n\t%s\n", buf);
        }
    }

    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);

    return 0;
}
