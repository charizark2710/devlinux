/*
| Property | flock | fcntl |
|---|---|---|
| Lock granularity | Whole file only | Byte range supported |
| Works over NFS | No | Yes |
| Inherited across fork | Yes | No |
| Best used when | Simple local file locking | Network FS or byte-range locking |
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/file.h>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s \"message text\"\n", argv[0]);
        return 1;
    }

    int fd = open("system.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1)
    {
        perror("Error opening system.log");
        return 1;
    }

    if (flock(fd, LOCK_EX) == -1)
    {
        perror("Error acquiring flock");
        close(fd);
        return 1;
    }

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", t);

    char buffer[256];
    int len = snprintf(buffer, sizeof(buffer), "[PID:%d] [%s] [INFO] %s\n", getpid(), time_str, argv[1]);

    if (len > 0)
    {
        write(fd, buffer, len);
    }
    flock(fd, LOCK_UN);
    close(fd);

    return 0;
}