#ifndef THREAD_QUEUE_H
#define THREAD_QUEUE_H

#include <pthread.h>

typedef struct {
    int id;
    pthread_cond_t *cond;
} thread_data;

typedef struct thread_queue thread_queue;

struct thread_queue {
    thread_data data;
    thread_queue *next;
};

thread_queue* queue_init();
void queue_destroy(thread_queue* first);

thread_data queue_enqueue(thread_queue *queue, pthread_cond_t *cond, int id);
thread_queue* queue_dequeue(thread_queue *first);

int queue_size(thread_queue *first);

#endif // THREAD_QUEUE_H
