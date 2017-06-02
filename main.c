#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include "thread_queue.h"
#include "bool.h"

#include "ulepszenia_terminala.h"

void client(pthread_cond_t *cond);
void print_queue(thread_queue *queue);
void dbg(thread_queue *queue);
void thread_start(void *val);

void monitor();

void barber();
void new_client();

int take_number();
int client_count;

pthread_t barber_thread;
pthread_t monitor_thread;
pthread_mutex_t waitroom = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t number = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t haircut = PTHREAD_MUTEX_INITIALIZER;

sem_t waiting_clients;
sem_t haircut_done;

int thread_count = 16;

thread_queue *clients;

int main(){
    srand(time(NULL));
    setbuf(stdout, NULL);
    clients = queue_init();
    sem_init(&waiting_clients, 0, 0);
    sem_init(&haircut_done, 0, 0);
    pthread_create(&barber_thread, NULL, (void*) barber, NULL);
    pthread_create(&monitor_thread, NULL, (void*) monitor, NULL);
    for(int i=0; i<thread_count; i++){
        int r = rand()%500 + 300;
        usleep(r*1000);
        new_client();
    }
    while(true){

    }
    pthread_exit(0);
}

void thread_start(void *val){
    pthread_cond_t *cond = (pthread_cond_t*) val;
    client(cond);
}

void barber(){
    while(true){
        printf("Barber is waiting for customers...\n");
        sem_wait(&waiting_clients);
        pthread_mutex_lock(&waitroom);
        printf("Barber wakes next customer\n");
        clients = queue_dequeue(clients);
        printf("Barber is doing haircut\n");
        pthread_cond_signal(clients->data.cond);
        pthread_mutex_unlock(&waitroom);
        sem_wait(&haircut_done);
        printf("Barber has done haircut\n");
    }
}

void new_client(){
    pthread_cond_t *cond = malloc(sizeof(pthread_cond_t));
    pthread_cond_init(cond, NULL);
    pthread_create(malloc(sizeof(pthread_t)), NULL, (void*) thread_start, (void*) cond);
}

void client(pthread_cond_t *cond){
    // go to waitroom
    pthread_mutex_lock(&waitroom);
    printf("New client arrives to waitroom...\n");
    //take number
    int id = take_number();
    printf("Client takes number: %d\n", id);
    queue_enqueue(clients, cond, id);
    sem_post(&waiting_clients);
    // wait
    printf("Client waits for haircut: %d\n", id);
    pthread_cond_wait(cond, &waitroom);
    pthread_mutex_unlock(&waitroom);

    pthread_mutex_lock(&haircut);
    printf("HAIRCUT: %d\n", id);
    sleep(1);
    sem_post(&haircut_done);
    pthread_mutex_unlock(&haircut);
}

int take_number(){
    pthread_mutex_lock(&number);
    client_count++;
    pthread_mutex_unlock(&number);
    return client_count;
}

void monitor(){
    while(true){
        int a, x, y=0, len;
        sem_getvalue(&waiting_clients, &a);
        if(a != 0) len = floor(log10(a)) + 1;
        else len = 1;
        x = 80 - (22 + len);
        UstawKursor(x, y++);
        printf(" +");
        for(int i=0; i<19+len; i++){
            printf("-");
        }
        printf("+");
        UstawKursor(x, y++);
        printf(" | Waiting clients: %d |", a);
        UstawKursor(x, y++);
        printf(" +");
        for(int i=0; i<19+len; i++){
            printf("-");
        }
        printf("+");
        UstawKursor(0, 24);
        usleep(10000);
    }
}

void print_queue(thread_queue *queue){
    while(queue->next != NULL){
        queue = queue->next;
        printf("%d, ", queue->data.id);
    }
    printf("\n");
}

void dbg(thread_queue *queue){
    printf("queue: ");
    print_queue(queue);
    printf("size: %d\n", queue_size(queue));
}
