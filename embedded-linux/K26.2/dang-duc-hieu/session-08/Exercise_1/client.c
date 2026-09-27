#include <ctype.h>
#include <string.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/monitor.sock"
#define BUFFER_SIZE 256

void trim_whitespace(char *str)
{
    size_t len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1]))
    {
        str[--len] = '\0';
    }
}

int main()
{
    int sock_fd;
    struct sockaddr_un server_addr;

    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd == -1)
    {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    // 3. Connect to the server socket path
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_un)) == -1)
    {
        perror("[monitor-cli] Connect failed. Is the server running?");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    printf("[monitor-cli] Connected to %s\n", SOCKET_PATH);

    while (1)
    {
        printf("> ");

        char command[10];
        if (scanf("%s", command) != 1)
        {
            printf("[monitor-cli] Invalid command \n");
        }
        ssize_t bytes_sent = write(sock_fd, command, strlen(command));
        if (bytes_sent == -1)
        {
            perror("Write to socket failed");
        }
        else
        {
            char buffer[BUFFER_SIZE];
            ssize_t bytes_received = recv(sock_fd, buffer, BUFFER_SIZE - 1, 0);
            if (bytes_received == 0)
            {
                printf("[monitor-cli] Server closed the connection gracefully.\n");
                return 0;
            }
            else if (bytes_received == -1)
            {
                perror("recv failed");
            }
            else
            {
                trim_whitespace(buffer);
                buffer[bytes_received] = '\0';
                printf("%s\n", buffer);
            }
        }
    }
    close(sock_fd);
    return 0;
}
