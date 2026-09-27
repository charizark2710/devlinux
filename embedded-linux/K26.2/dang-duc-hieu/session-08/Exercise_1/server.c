#include <ctype.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/monitor.sock"
#define BUFFER_SIZE 256

void connect_client(int server_fd, int *client_fd)
{
    *client_fd = accept(server_fd, NULL, NULL);
    if (*client_fd == -1)
    {
        perror("accept error");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("[Daemon] Client connected. \n");
}

void send_to_client(int client_fd, char *message)
{
    ssize_t bytes_sent = send(client_fd, message, strlen(message), MSG_NOSIGNAL);
    if (bytes_sent == -1)
    {
        perror("Send failed");
    }
}

char *read_memproc()
{
    char line[128];
    unsigned long long mem_total = 0;
    unsigned long long mem_free = 0;

    FILE *f = fopen("/proc/meminfo", "r");
    if (!f)
    {
        perror("Failed to open /proc/meminfo");
        char *err_buf = NULL;
        asprintf(&err_buf, "ERROR: Unable to read memory info\n");
        return err_buf;
    }

    while (fgets(line, sizeof(line), f))
    {
        if (!mem_total && strncmp(line, "MemTotal:", 9) == 0)
        {
            sscanf(line, "MemTotal: %llu kB", &mem_total);
        }
        else if (!mem_free && strncmp(line, "MemFree:", 8) == 0)
        {
            sscanf(line, "MemFree: %llu kB", &mem_free);
        }

        if (mem_free && mem_total)
        {
            break;
        }
    }
    fclose(f);

    char *buffer = NULL;
    asprintf(&buffer, "mem_total=%llu kB mem_free=%llu kB", mem_total, mem_free);
    return buffer;
}

float read_cpu_usage()
{
    FILE *fp = fopen("/proc/loadavg", "r");
    if (fp == NULL)
    {
        perror("Failed to open /proc/loadavg");
        return EXIT_FAILURE;
    }

    float load;

    if (fscanf(fp, "%f",
               &load) < 1)
    {
        exit(EXIT_FAILURE);
    }

    fclose(fp);
    return load;
}

void handle_sigint()
{
    unlink(SOCKET_PATH);
    exit(0);
}

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
    int server_fd;
    int client_fd = -1;
    struct sockaddr_un addr;
    char buffer[BUFFER_SIZE];

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    unlink(SOCKET_PATH);

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1)
    {
        perror("bind error");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) == -1)
    {
        perror("listen error");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, handle_sigint);

    printf("[Daemon] Listening on %s...\n", SOCKET_PATH);

    while (1)
    {
        if (client_fd < 0)
        {
            connect_client(server_fd, &client_fd);
        }

        ssize_t bytes_read = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_read > 0)
        {
            buffer[bytes_read] = '\0';
            trim_whitespace(buffer);
            if (strcmp(buffer, "cpu") == 0)
            {
                float cpu = read_cpu_usage();
                char *res = NULL;
                asprintf(&res, "load_avg=%f", cpu);

                send_to_client(client_fd, res);
                free(res);
            }
            else if (strcmp(buffer, "mem") == 0)
            {
                char *res = read_memproc();
                send_to_client(client_fd, res);
                free(res);
            }
            else if (strcmp(buffer, "quit") == 0)
            {
                printf("Client disconnected. Waiting for next client...\n");
                close(client_fd);
                client_fd = -1;
            }
            else
            {
                send_to_client(client_fd, "ERROR: unknown command");
            }
        }
        else
        {
            printf("Client disconnected. Waiting for next client...\n");
            close(client_fd);
            client_fd = -1;
        }
    }
}
