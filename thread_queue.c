#include "thread_queue.h"

#include <stdlib.h>

thread_data queue_enqueue(thread_queue *queue, pthread_cond_t *cond, int id){
    while(queue->next != NULL){
        queue = queue->next;
    }
    queue->next = malloc(sizeof(thread_queue));

    queue = queue->next;
    queue->next = NULL;
    queue->data.id = id;
    queue->data.cond = cond;
    return queue->data;
}

thread_queue *queue_dequeue(thread_queue *first){
    thread_queue *second = first->next;
    //pthread_cond_destroy(first->data.cond);
    free(first);
    return second;
}

int queue_size(thread_queue *first){
    int size = 0;
    while(first->next != NULL){
        first = first->next;
        size++;
    }
    return size;
}

thread_queue *queue_init(){
    thread_queue *queue = malloc(sizeof(thread_queue));
    queue->next = NULL;
    return queue;
}

void queue_destroy(thread_queue *first){
    while(first->next != NULL){
        first = queue_dequeue(first);
    }
    free(first);
}
