// Add at top of book_ticket function:
/*
 * CRITICAL SECTION ATOMICITY EXPLANATION:
 * The check (seats_available >= wanted) and deduct (seats_available -= wanted)
 * MUST be inside the SAME lock/unlock block. If split into 2 separate lock
 * acquisitions, a race condition occurs:
 *   Thread A: lock → check (5 seats ok) → unlock
 *   Thread B: lock → check (5 seats ok) → unlock
 *   Thread A: lock → deduct (5-3=2) → unlock
 *   Thread B: lock → deduct (2-4=-2 INVALID!) → unlock
 * By wrapping both in same critical section, atomicity is guaranteed.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

typedef struct
{
    int agent_id;
    char customer[50];
    int seats_wanted;
} BookingRequest;

pthread_mutex_t seat_lock;
int seats_available = 10;
int failed_bookings = 0;
int req_count = 0;

void *book_ticket(void *arg)
{
    BookingRequest *request = (BookingRequest *)arg;

    printf("[Agent %d | TID %lu...] Booking %d seat(s) for %s...\n",
           request->agent_id, (unsigned long)pthread_self(), request->seats_wanted, request->customer);

    __sync_add_and_fetch(&req_count, 1);
    if (__sync_fetch_and_add(&req_count, 0) == 5)
    {
        printf("--- [all agents reach critical section] ---\n");
    }

    sleep(1);
    pthread_mutex_lock(&seat_lock);

    if (seats_available >= request->seats_wanted)
    {
        seats_available -= request->seats_wanted;
        printf("[Agent %d] CONFIRMED: %d seat(s) for %s. Remaining: %d\n",
               request->agent_id, request->seats_wanted, request->customer, seats_available);
    }
    else
    {
        failed_bookings++;
        printf("[Agent %d] SOLD OUT: needs %d seat(s), only %d left — booking failed.\n",
               request->agent_id, request->seats_wanted, seats_available);
    }

    pthread_mutex_unlock(&seat_lock);
    return NULL;
}

int main(int argc, char const *argv[])
{
    BookingRequest requests[5] = {
        {1, "Nguyen Van An", 2},
        {2, "Tran Thi Bich", 1},
        {3, "Le Van Cuong", 3},
        {4, "Pham Thi Dung", 1},
        {5, "Hoang Van Em", 2}};

    pthread_mutex_init(&seat_lock, NULL);
    pthread_t threads[5];

    printf("==============================================\n");
    printf("   TICKET BOOKING SYSTEM (5 agents, 10 seats) \n");
    printf("==============================================\n");

    for (int i = 0; i < 5; i++)
    {
        pthread_create(&threads[i], NULL, book_ticket, (void *)&requests[i]);
    }

    for (int i = 0; i < 5; i++)
    {
        pthread_join(threads[i], NULL);
    }

    printf("\n================ SUMMARY ================\n");
    printf("  Total seats     : 10\n");
    printf("  Seats sold      : %d\n", 10 - seats_available);
    printf("  Seats remaining : %d\n", seats_available);
    printf("  Failed bookings : %d\n", failed_bookings);
    printf("=========================================\n");

    pthread_mutex_destroy(&seat_lock);
    return 0;
}