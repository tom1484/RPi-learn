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

int main(int argc, char *argv[]) {
    // Parse target ip address and port from arguments.
    if (argc < 3) {
        printf("invalid arguments [<ip> <port>]\n");
        exit(EXIT_FAILURE);
    }

    // Creact socket.
    int socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_fd == -1) {
        perror("cannot create socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in st_sockaddr;
    size_t addr_len = sizeof(struct sockaddr_in);
    memset(&st_sockaddr, 0, sizeof(struct sockaddr_in));

    // Configure socket info..
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

    // Send connection request.
    if (connect(socket_fd, (struct sockaddr *)&st_sockaddr, addr_len) < 0) {
        perror("error connect failed");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }
    printf("connected\n");

    int count = 0;
    char action[32];
    int com_num;

    char msg[1024];
    char buf[1024];
    size_t read_len;
    while (1) {
        // Exit if error occurred.
        if ((read_len = recv(socket_fd, buf, sizeof(buf), 0)) < 0) {
            perror("error read failed");
            close(socket_fd);
            exit(EXIT_FAILURE);
        }
        // Disconnect if received nothing.
        else if (read_len == 0) {
            printf("disconnected\n");
            break;
        }
        else {
            buf[read_len] = '\0';

            // Separate action and number by space.
            char *ptr = strtok(buf, " ");
            strcpy(action, ptr);

            ptr = strtok(NULL, " ");
            if (ptr == NULL) {
                sprintf(msg, "invalid command");
            }
            else {
                // Parse number from string.
                char *end;
                com_num = strtol(ptr, &end, 10);
                if (end == ptr || errno != 0) {
                    sprintf(msg, "invalid number");
                }
                else {
                    // Check if action is valid and execute if it is.
                    bool com_exist = 1;
                    if (strcmp(action, "add") == 0) {
                        count += com_num;
                    }
                    else if (strcmp(action, "sub") == 0) {
                        count -= com_num;
                    }
                    else if (strcmp(action, "mul") == 0) {
                        count *= com_num;
                    }
                    else if (strcmp(action, "div") == 0) {
                        count /= com_num;
                    }
                    else {
                        com_exist = false;
                        sprintf(msg, "invalid action");
                    }

                    if (com_exist) {
                        sprintf(msg, "%d", count);
                        printf("count: %d\n", count);
                    }
                }
            }

            send(socket_fd, msg, strlen(msg), 0);
        }
    }

    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);

    return 0;
}
