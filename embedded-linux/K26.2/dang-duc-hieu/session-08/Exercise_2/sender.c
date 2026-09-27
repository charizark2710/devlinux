#include <string.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define COLLECTOR_IP "127.0.0.1"
#define COLLECTOR_PORT 9000
#define BUFFER_SIZE 128

double calc_mem_usage()
{
    char line[128];
    unsigned long long mem_total = 0;
    unsigned long long mem_free = 0;

    FILE *f = fopen("/proc/meminfo", "r");
    if (!f)
    {
        perror("Failed to open /proc/meminfo");
        return 0;
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

    return (double)(mem_total - mem_free) / mem_total * 100.0;
}

float calc_temp()
{
    FILE *fp = fopen("/proc/loadavg", "r");
    if (fp == NULL)
    {
        perror("Failed to open /proc/loadavg");
        return -1.0f;
    }

    float load = 0.0f;
    if (fscanf(fp, "%f", &load) < 1)
    {
        fclose(fp);
        return -1.0f;
    }

    fclose(fp);
    return 40.0f + load * 10.0f;
}

int main()
{
    int sock_fd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd == -1)
    {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(COLLECTOR_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    if (inet_aton(COLLECTOR_IP, &server_addr.sin_addr) == 0)
    {
        fprintf(stderr, "Invalid IP address: %s\n", COLLECTOR_IP);
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
    printf("[Sensor] Target collector: %s\n", COLLECTOR_IP);

    for (int i = 0; i < 5; i++)
    {
        double mem_usage = calc_mem_usage();
        float temp = calc_temp();

        int len = sprintf(buffer, "id=sensor-01 temp=%f mem_used=%f%%", temp, mem_usage);
        sendto(sock_fd, (const char *)buffer, len, 0,
               (const struct sockaddr *)&server_addr, sizeof(server_addr));

        printf("[Sent %d/5] id=sensor-01 temp=%f mem_used=%f%%]\n", i + 1, temp, mem_usage);
        sleep(2);
    }
    close(sock_fd);
    return 0;
}
