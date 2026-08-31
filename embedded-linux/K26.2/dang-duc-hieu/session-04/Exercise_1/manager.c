#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

typedef struct
{
    int id;
    char name[50];
    int quantity;
    float unit_price;
} Order;

float process_order(Order o)
{
    float total = o.quantity * o.unit_price;
    printf("[CHILD-%d] PID: %d | PPID: %d\n", o.id, getpid(), getppid());
    printf("[CHILD-%d] %s x%d — Total: %.0f VND\n",
           o.id, o.name, o.quantity, total);
    printf("[CHILD-%d] Processing... (sleep 2s)\n\n", o.id);
    sleep(2);
    return total;
}

int main()
{
    Order orders[3] = {
        {1, "Backpack", 2, 350000},
        {2, "Shoes", 1, 500000},
        {3, "Hat", 3, 120000}};
    size_t length = sizeof(orders) / sizeof(orders[0]);

    int pids[length];

    int pipefd[2];

    if (pipe(pipefd) == -1)
    {
        return 1;
    }

    for (size_t i = 0; i < length; i++)
    {
        fflush(stdout);
        int pid = fork();
        if (pid < 0)
        {
            perror("fork failed");
            exit(1);
        }
        if (pid == 0)
        {
            close(pipefd[0]);

            float order_total = process_order(orders[i]);
            ssize_t n = write(pipefd[1], &order_total, sizeof(order_total));
            if (n == -1)
                perror("write error");
            close(pipefd[1]);
            exit(0);
        }
        else if (pid > 0)
        {
            pids[i] = pid;
            printf("[MANAGER] fork() order #%zu → child PID: %d\n", i + 1, pid);
        }
    }

    close(pipefd[1]);
    int successful = 0;
    int failed = 0;
    printf("===================================================\n");
    printf("   ORDER PROCESSING SYSTEM — MANAGER (fork+wait)\n");
    printf("===================================================\n");
    printf("[MANAGER] PID: %d — spawning 3 child processes...\n\n", getpid());
    for (size_t i = 0; i < length; i++)
    {
        int status;
        pid_t wpid = waitpid(pids[i], &status, 0);

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
        {
            successful++;
            printf("[MANAGER] waitpid(%d) — order #%d: exit code=%d → SUCCESS \n", wpid, orders[i].id, WEXITSTATUS(status));
        }
        else
        {
            failed++;
        }
    }
    printf("\n--- [~2 seconds later, all %zu children call exit(0)] ---\n\n", length);

    float total_revenue = 0;
    float current_total = 0;
    while (read(pipefd[0], &current_total, sizeof(current_total)) > 0)
    {
        total_revenue += current_total;
    }
    close(pipefd[0]);

    printf("================= SUMMARY =================\n");
    printf("  Total orders    : %zu\n", length);
    printf("  Successful      : %d\n", successful);
    printf("  Failed          : %d\n", failed);
    printf("  Total revenue   : %.0f VND\n", total_revenue);
    printf("===========================================\n");

    return 0;
}