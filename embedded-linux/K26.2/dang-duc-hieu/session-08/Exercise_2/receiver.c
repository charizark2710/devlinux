#include <string.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>

#define COLLECTOR_PORT 9000
#define BUFFER_SIZE 128

int main()
{
    int sock_fd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd == -1)
    {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(COLLECTOR_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    if (bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in)) == -1)
    {
        perror("bind error");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
    printf("[Collector] Listening on %s:%d...\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));

    while (1)
    {
        socklen_t addr_len = sizeof(client_addr);

        int len = recvfrom(sock_fd, buffer, BUFFER_SIZE - 1, 0,
                           (struct sockaddr *)&client_addr, &addr_len);
        if (len < 0)
        {
            perror("Receive failed");
            break;
        }

        buffer[len] = '\0';

        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char ts[16];
        strftime(ts, sizeof(ts), "%H:%M:%S", t);
        printf("[%s] %s:%d → %s\n", ts,
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buffer);
    }
    close(sock_fd);
    return 0;
}
