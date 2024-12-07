#include "sched.h"
#include "switch.h"
#include <stdlib.h>
#include <limits.h>
#include <pthread.h>

extern struct proc proc[];
extern int nprocs;

#define MAX_PRIORITY 2

// Listas para MLFQ
list_t ready_queues[MAX_PRIORITY + 1];
list_t rr_queue;

void init_scheduler() {
    for (int i = 0; i <= MAX_PRIORITY; i++) {
        List_Create(&ready_queues[i]);
    }
    List_Create(&rr_queue);
}

// Selección de procesos para SJF
struct proc* select_next_sjf() {
    struct proc* shortest = NULL;
    int min_burst = INT_MAX;

    for (int i = 0; i < nprocs; i++) {
        if (proc[i].status == READY && proc[i].burst_time < min_burst) {
            min_burst = proc[i].burst_time;
            shortest = &proc[i];
        }
    }
    return shortest;
}

// Selección de procesos para MLFQ
struct proc* select_next_mlfq() {
    for (int p = 0; p <= MAX_PRIORITY; p++) {
        list_t *queue = &ready_queues[p];
        pthread_mutex_lock(&queue->lock);

        if (queue->head != NULL) {
            node_t *node = queue->head;
            queue->head = queue->head->next;
            pthread_mutex_unlock(&queue->lock);

            return &proc[node->key];
        }

        pthread_mutex_unlock(&queue->lock);
    }
    return NULL;
}

// Selección de procesos para Round Robin
struct proc* select_next_rr() {
    pthread_mutex_lock(&rr_queue.lock);

    if (rr_queue.head != NULL) {
        node_t *node = rr_queue.head;
        rr_queue.head = rr_queue.head->next;

        if (proc[node->key].status == READY) {
            node_t *tail = node;
            while (tail->next != NULL) {
                tail = tail->next;
            }
            tail->next = node;
            node->next = NULL;
        }

        pthread_mutex_unlock(&rr_queue.lock);
        return &proc[node->key];
    }

    pthread_mutex_unlock(&rr_queue.lock);
    return NULL;
}

// Scheduler principal
void scheduler() {
    while (!done()) {
        struct proc *candidate = select_next_rr(); // Cambiar a select_next_sjf() o select_next_mlfq()

        if (candidate != NULL) {
            candidate->status = RUNNING;
            candidate->runtime += swtch(candidate);
            candidate->status = READY;
        } else {
            idle();
        }
    }
}
