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

int digit_count(int n);
int bigger(int a, int b);

int take_number();
int client_count;
int resign_count;
int served = -1;

pthread_t barber_thread;
pthread_t monitor_thread;
pthread_mutex_t waitroom = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t number = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t haircut = PTHREAD_MUTEX_INITIALIZER;

sem_t waiting_clients;
sem_t haircut_done;
sem_t waitroom_seats;

int waitroom_limit = 4;

const int verbose = 2;

thread_queue *clients;

int main(){
    srand(time(NULL));
    setbuf(stdout, NULL);
    clients = queue_init();
    sem_init(&waiting_clients, 0, 0);
    sem_init(&haircut_done, 0, 0);
    sem_init(&waitroom_seats, 0, waitroom_limit);
    pthread_create(&barber_thread, NULL, (void*) barber, NULL);
    pthread_create(&monitor_thread, NULL, (void*) monitor, NULL);
    while(true){
        ZmienTryb(true);
        char a = getchar();
        if(a == 'c') new_client();
    }
    pthread_exit(0);
}

void thread_start(void *val){
    pthread_cond_t *cond = (pthread_cond_t*) val;
    client(cond);
}

void barber(){
    while(true){
        if(verbose > 0) printf("Barber is waiting for customers...\n");
        sem_wait(&waiting_clients);
        pthread_mutex_lock(&waitroom);
        if(verbose > 1)  printf("Barber wakes next customer\n");
        clients = queue_dequeue(clients);
        if(verbose > 1)  printf("Barber is doing a haircut\n");
        pthread_cond_signal(clients->data.cond);
        pthread_mutex_unlock(&waitroom);
        sem_wait(&haircut_done);
        if(verbose > 1)  printf("Barber has done a haircut\n");
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
    //take number
    int id = take_number();
    if(verbose > 0) printf("New client takes number: %d\n", id);
    if(sem_trywait(&waitroom_seats) != 0){
        if(verbose > 0) printf("There's no free places! Client %d goes out\n", id);
        resign_count++;
        pthread_mutex_unlock(&waitroom);
        return;
    }
    queue_enqueue(clients, cond, id);
    sem_post(&waiting_clients);
    // wait
    if(verbose > 1)  printf("Client %d waits for haircut\n", id);
    pthread_cond_wait(cond, &waitroom);
    sem_post(&waitroom_seats);
    pthread_mutex_unlock(&waitroom);

    pthread_mutex_lock(&haircut);
    served = id;
    if(verbose > 0) printf("Client %d is getting a haircut\n", id);
    usleep(1000*3000);
    sem_post(&haircut_done);
    served = -1;
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
        int waiting, x = 80, y=0;
        sem_getvalue(&waiting_clients, &waiting);

        int served_len = 20, waiting_len = 23, resign_len = 21;

        if(served == -1) served_len = 0;
        else served_len += digit_count(served);

        waiting_len += digit_count(waiting) + digit_count(waitroom_limit);
        resign_len += digit_count(resign_count);

        int len = bigger(served_len, bigger(waiting_len, resign_len));
        x -= len;

        UstawKursor(x, y++);
        printf(" +");
        for(int i=0; i<len-3; i++) printf("-");
        printf("+");

        if(served != -1){
            UstawKursor(x, y++);
            printf(" | Served client: %*s%d |", len-served_len, "", served);
        }

        UstawKursor(x, y++);
        printf(" | Waiting clients: %*s%d/%d |", len-waiting_len, "", waiting, waitroom_limit);

        UstawKursor(x, y++);
        printf(" | Resigned count: %*s%d |", len-resign_len, "", resign_count);

        UstawKursor(x, y++);
        printf(" +");
        for(int i=0; i<len-3; i++) printf("-");
        printf("+");

        UstawKursor(x, y++);
        for(int i=0; i<len; i++) printf(" ");

        UstawKursor(0, 24);
        usleep(10000);
    }
}

int digit_count(int n){
    int count;
    if(n != 0) count = floor(log10(n)) + 1;
    else count = 1;
    return count;
}

int bigger(int a, int b){
    return a > b ? a : b;
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
