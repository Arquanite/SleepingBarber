#ifndef THREAD_QUEUE_H
#define THREAD_QUEUE_H

#include <pthread.h>

typedef struct thread_queue thread_queue;

struct thread_queue {
    int id;
    thread_queue *next;
};

thread_queue* queue_init();
void queue_destroy(thread_queue* first);

int queue_enqueue(thread_queue *queue, int id);
thread_queue* queue_dequeue(thread_queue *first);

int queue_size(thread_queue *first);

#endif // THREAD_QUEUE_H
