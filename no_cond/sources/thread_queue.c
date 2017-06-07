#include "thread_queue.h"

#include <stdlib.h>

int queue_enqueue(thread_queue *queue, int id){
    while(queue->next != NULL){
        queue = queue->next;
    }
    queue->next = malloc(sizeof(thread_queue));

    queue = queue->next;
    queue->next = NULL;
    queue->id = id;
    return id;
}

thread_queue *queue_dequeue(thread_queue *first){
    thread_queue *second = first->next;
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
