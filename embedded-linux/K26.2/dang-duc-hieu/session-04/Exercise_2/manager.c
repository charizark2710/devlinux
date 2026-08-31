#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

typedef struct
{
    char id[10];
    char name[50];
    int quantity;
    float unit_price;
} Student;

int main()
{
    printf("=============================================\n");
    printf("   STUDENT LOOKUP SYSTEM — MANAGER\n");
    printf("   (fork + execve | file: students.txt)\n");
    printf("=============================================\n");
    printf("[MANAGER] PID: %d\n", getpid());
    printf("Enter student ID ('quit' to exit).\n\n");
    printf("---------------------------------------------\n");

    while (1)
    {
        char cmd[50];
        if (scanf("%49s", cmd) != 1)
        {
            printf("Input error\n");
            continue;
        }
        if (strcmp(cmd, "q") == 0 || strcmp(cmd, "quit") == 0)
        {
            printf("[MANAGER] Exiting. Goodbye!\n");
            break;
        }

        int childPid = 0;

        int pid = fork();

        if (pid < 0)
        {
            perror("fork failed");
            continue;
        }

        if (pid == 0)
        {
            // add searcher as argv[0] for program name
            char *args[] = {"./searcher", cmd, "./students.txt", NULL};
            childPid = pid;
            execve("./searcher", args, NULL);

            // Reached only if execve fails
            perror("[CHILD] execve failed");
            exit(2);
        }
        if (pid > 0)
        {

            printf("\n[MANAGER] fork() → child PID: %d\n", pid);
            printf("[MANAGER] Waiting for child (waitpid)...\n\n");

            int status;
            int wpid = waitpid(pid, &status, 0);
            if (WIFEXITED(status))
            {
                int exit_code = WEXITSTATUS(status);
                char *status_msg = (exit_code == 0) ? "Found" : "Not found";

                printf("\n[MANAGER] Child (PID %d) exited. code=%d → %s\n\n",
                       wpid, exit_code, status_msg);
            }
        }
    }
    return 0;
}