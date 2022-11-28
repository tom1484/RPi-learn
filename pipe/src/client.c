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
#include <stdbool.h>

int main(int argc, char *argv[])
{
    // Parse target ip address and port from arguments.
    if (argc < 2)
    {
        printf("invalid arguments [<port>]\n");
        exit(EXIT_FAILURE);
    }

    // Creact socket.
    int socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_fd == -1)
    {
        perror("cannot create socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in st_sockaddr;
    size_t addr_len = sizeof(struct sockaddr_in);
    memset(&st_sockaddr, 0, sizeof(struct sockaddr_in));

    // Configure socket info..
    // char *ip_address = argv[1];
    char *ip_address = "127.0.0.1";
    st_sockaddr.sin_family = PF_INET;
    // st_sockaddr.sin_port = htons(atoi(argv[2]));
    // st_sockaddr.sin_port = htons(1484);
    st_sockaddr.sin_port = htons(atoi(argv[1]));
    int res = inet_pton(PF_INET, ip_address, &st_sockaddr.sin_addr);

    if (res < 0)
    {
        perror("error: first parameter is not a valid address family");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }
    else if (0 == res)
    {
        perror("char string (second parameter does not contain valid ipaddress");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    // Send connection request.
    if (connect(socket_fd, (struct sockaddr *)&st_sockaddr, addr_len) < 0)
    {
        perror("error connect failed");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }
    printf("connected\n\n");

    int count = 0;
    char action[32];
    int com_num;

    char msg[1024];
    char buf[1024];
    size_t read_len;
    while (1)
    {
        // printf("command:\n");
        fgets(msg, (int)sizeof(msg), stdin);
        msg[strlen(msg) - 1] = '\0';
        send(socket_fd, msg, strlen(msg), 0);

        if (recv(socket_fd, buf, 1024, 0) > 0)
        {
            // printf("result:\n%s\n", buf);
            printf("\n%s\n", buf);
        }
    }

    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);

    return 0;
}
