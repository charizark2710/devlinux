/*
 * SPURIOUS WAKEUP AND WHILE LOOP:
 * A condition variable can wake up even if the condition wasn't signaled
 * (spurious wakeup). Using 'while' instead of 'if' is essential:
 *
 *   WRONG (using if):
 *   if (count == 0 && !all_sent)
 *       pthread_cond_wait(&not_empty, &q_lock);
 *   // Between wakeup and reacquiring mutex, another thread may dequeue
 *   dequeue(doc);  // BUG: count may now be 0, accessing NULL
 *
 *   CORRECT (using while):
 *   while (count == 0 && !all_sent)
 *       pthread_cond_wait(&not_empty, &q_lock);
 *   // After reacquiring mutex, condition is re-checked
 *   dequeue(doc);  // SAFE: count is guaranteed > 0
 *
 * Spurious wakeup = kernel may wake thread even without signal.
 * POSIX allows this optimization. Always use while loop with cond_wait.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

typedef struct
{
    int doc_id;
    char filename[60];
    int pages;
} Document;

typedef struct
{
    int producer_id;
    Document *docs;
} ProducerArgs;

Document *queue[5];
int head = 0, tail = 0, count = 0;
int all_sent = 0;

// Summary counters
int total_submitted = 0;
int total_printed = 0;
int total_pages_printed = 0;

pthread_mutex_t q_lock;
pthread_cond_t not_full;  /* producers wait here when count == 5 */
pthread_cond_t not_empty; /* printer waits here when count == 0 */

void *producer(void *arg)
{
    ProducerArgs *pargs = (ProducerArgs *)arg;

    for (int i = 0; i < 3; i++)
    {
        Document *doc = &pargs->docs[i];

        /*
         * CRITICAL: Use WHILE loop, NOT if statement.
         * Reason: Spurious wakeup — pthread_cond_wait() có thể return mà không được signaled
         * (do OS scheduling, signal interrupt, v.v.). Nếu dùng if, ta sẽ assume condition đã
         * thỏa mà thực tế chưa → dequeue rỗng, crash.
         * While loop re-check condition sau khi wakeup → đảm bảo condition thực sự thỏa.
         */
        pthread_mutex_lock(&q_lock);

        while (count == 5)
        {
            printf("[Producer %d] Queue full — waiting...\n", pargs->producer_id);
            pthread_cond_wait(&not_full, &q_lock);
        }

        queue[tail] = doc;
        tail = (tail + 1) % 5;
        count++;
        total_submitted++;

        printf("[Producer %d] Submitting: %-15s (%2d pages) — queue: %d/5\n",
               pargs->producer_id, doc->filename, doc->pages, count);

        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&q_lock);

        sleep(1);
    }
    return NULL;
}

void *printer(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&q_lock);

        while (count == 0 && !all_sent)
        {
            pthread_cond_wait(&not_empty, &q_lock);
        }

        if (count == 0 && all_sent)
        {
            pthread_mutex_unlock(&q_lock);
            break;
        }

        Document *doc = queue[head];
        queue[head] = NULL;
        head = (head + 1) % 5;
        count--;

        total_printed++;
        total_pages_printed += doc->pages;

        printf("[Printer]    Printing:   %-15s (%2d pages) — queue: %d/5\n",
               doc->filename, doc->pages, count);

        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&q_lock);

        sleep(1); // Simulate printing time
    }

    printf("[Printer]    All documents printed. Exiting.\n");
    return NULL;
}

int main(int argc, char const *argv[])
{
    Document docs[9] = {
        // Producer 1
        {1, "report_Q1.pdf", 12},
        {2, "slides.pdf", 20},
        {3, "summary.pdf", 4},

        // Producer 2
        {4, "contract.pdf", 5},
        {5, "memo.pdf", 2},
        {6, "budget.pdf", 7},

        // Producer 3
        {7, "invoice.pdf", 3},
        {8, "proposal.pdf", 8},
        {9, "appendix.pdf", 5}};

    printf("==============================================\n");
    printf("   OFFICE PRINT QUEUE (3 producers, 1 printer)\n");
    printf("   Queue capacity: 5 documents\n");
    printf("==============================================\n\n");

    pthread_mutex_init(&q_lock, NULL);
    pthread_cond_init(&not_full, NULL);
    pthread_cond_init(&not_empty, NULL);

    pthread_t producer_threads[3];
    pthread_t printer_thread;

    ProducerArgs pargs[3] = {
        {1, &docs[0]},
        {2, &docs[3]},
        {3, &docs[6]}};

    pthread_create(&printer_thread, NULL, printer, NULL);

    for (int i = 0; i < 3; i++)
    {
        pthread_create(&producer_threads[i], NULL, producer, (void *)&pargs[i]);
    }

    for (int i = 0; i < 3; i++)
    {
        pthread_join(producer_threads[i], NULL);
    }

    // Wake up printer thread in case it is waiting on empty queue
    pthread_mutex_lock(&q_lock);
    all_sent = 1;
    pthread_cond_signal(&not_empty);
    pthread_mutex_unlock(&q_lock);

    pthread_join(printer_thread, NULL);

    printf("\n================ SUMMARY ================\n");
    printf("  Documents submitted : %d\n", total_submitted);
    printf("  Documents printed   : %d\n", total_printed);
    printf("  Total pages printed : %d\n", total_pages_printed);
    printf("=========================================\n");

    pthread_cond_destroy(&not_full);
    pthread_cond_destroy(&not_empty);
    pthread_mutex_destroy(&q_lock);

    return 0;
}