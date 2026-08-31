#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <stdlib.h>

ssize_t read_line(int fd, char *buffer, size_t max_len)
{
    size_t bytes_read = 0;
    char ch;

    while (bytes_read < max_len - 1)
    {
        ssize_t result = read(fd, &ch, 1);
        if (result <= 0)
        {
            if (bytes_read == 0)
                return result;
            break;
        }
        if (ch == '\n')
            break;

        buffer[bytes_read++] = ch;
    }

    buffer[bytes_read] = '\0';
    return bytes_read;
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("[SEARCHER] Only receive %d args", argc);
        return 2;
    }

    char *target_id = argv[1];
    char *filepath = argv[2];
    printf("[SEARCHER] PID: %d | PPID: %d\n", getpid(), getppid());
    printf("[SEARCHER] Searching for \"%s\" in %s...\n", target_id, filepath);
    fflush(stdout);

    int fd = open(filepath, O_RDONLY, 0644);

    if (fd == -1)
    {
        fprintf(stderr, "[SEARCHER ERROR] Cannot open file '%s'\n", filepath);
        perror("[SEARCHER ERROR] Details");
        // perror("[SEARCHER] File error");
        return 2;
    }

    if (flock(fd, LOCK_SH) == -1)
    {
        perror("[SEARCHER] Error acquiring flock");
        close(fd);
        return 1;
    }

    char line[100];
    int found = 0;

    while (read_line(fd, line, sizeof(line)) > 0)
    {
        if (strlen(line) == 0)
            continue;

        char *id = strtok(line, "|");
        char *name = strtok(NULL, "|");
        char *class_name = strtok(NULL, "|");
        char *gpa_str = strtok(NULL, "|");

        if (strcmp(id, target_id) == 0)
        {
            float gpa = atoi(gpa_str);

            char *grade = "Average";
            if (gpa >= 9.0)
                grade = "Excellent";
            else if (gpa >= 8.0)
                grade = "Good";
            else if (gpa >= 6.5)
                grade = "Fair";

            printf("\n========== SEARCH RESULT ==========\n");
            printf("  ID      : %s\n", id);
            printf("  Name    : %s\n", name);
            printf("  Class   : %s\n", class_name);
            printf("  GPA     : %.1f\n", gpa);
            printf("  Grade   : %s\n", grade);
            printf("====================================\n");
            found = 1;
            break;
        }
    }

    flock(fd, LOCK_UN);
    close(fd);

    if (!found)
    {
        printf("[SEARCHER] No student found with ID: %s\n", target_id);
        return 1;
    }

    return 0;
}