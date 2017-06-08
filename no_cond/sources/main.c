#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <string.h>
#include "thread_queue.h"
#include "bool.h"

#include "ulepszenia_terminala.h"

// Funkcja początkowa klienta - konwersja parametrów
void thread_start(void *val);

// Funkcja główna klienta - poczekalnia, strzyżenie itp.
void client();

// Funkcja przydzielająca numery klientom (kasa biletowa)
int take_number();

// Rysuje okienko z podstawowymi informacjami o działaniu programu
void frame_stats();

// Wypisuje podstawowe informacje o działaniu programu
void print_stats();

// Wypisuje kolejkę (i jej nazwę)
void print_queue(thread_queue *queue, const char *name);

// Funkcja fryzjera
void barber();

// Funkcja tworząca nowy wątek klienta
void new_client();

// Funkcja zwracająca długość podanej liczby
int digit_count(int n);

// Funkcja zwracająca większą z liczb
int bigger(int a, int b);

// Parsowanie opcji
void parse_args(int argc, char *argv[]);

// Funkcja zamieniająca liczbę ze stringa na int
int parse_int(const char *number);

// Liczba wszystkich klientów, którzy postanowili pójść do fryzjera
int client_count;

// Liczba klientów, którzy zrezygnowali
int resign_count;

// Numer aktualnie obsługiwanego klienta
int served_client = -1;

pthread_t barber_thread; // Wątek fryzjera
pthread_t frame_thread; // Wątek ramki

// Mutex zabezpieczający poczekalnie
pthread_mutex_t waitroom = PTHREAD_MUTEX_INITIALIZER;

// Mutex zabezpieczający kasę biletową
pthread_mutex_t number = PTHREAD_MUTEX_INITIALIZER;

// Mutex zabezpieczający gabinet
pthread_mutex_t haircut = PTHREAD_MUTEX_INITIALIZER;

sem_t waiting_clients; // Semafor zliczający oczekujących klientów
sem_t haircut_done; // Semafor informujący o zakończeniu strzyżenia
sem_t waitroom_seats; // Semafor zliczający wolne miejsca w poczekalni
sem_t barber_ready; // Semafor informujący o gotowości fryzjera (budzi klienta)

// Liczba miejsc w poczekalni
int waitroom_limit = 4;

// Długość strzyżenia (w milisekundach)
int haircut_time = 3000;

// Poziom informowania o działaniu programu
int verbose = 1;

// Poziom (styl) informacji debugowych
int debug = 0;

// Kolejka klientów
thread_queue *clients;

// Kolejka (lista) klientów, którzy zrezygnowali
thread_queue *resigned;

int main(int argc, char *argv[]){
    srand(time(NULL));
    setbuf(stdout, NULL);
    parse_args(argc, argv);
    printf("CONFIG: verbose level: %d, debug level: %d, waitroom size: %d, haircut: %dms\n"
           , verbose, debug, waitroom_limit, haircut_time);
    printf("Press c to add clients, q to exit\n\n");

    // Inicjalizacja kolejek
    clients = queue_init();
    resigned = queue_init();

    // Inicjalizacja semaforów
    sem_init(&waiting_clients, 0, 0);
    sem_init(&haircut_done, 0, 0);
    sem_init(&waitroom_seats, 0, waitroom_limit);
    sem_init(&barber_ready, 0, 0);

    // Tworzenie wątku fryzjera
    pthread_create(&barber_thread, NULL, (void*) barber, NULL);

    if(debug > 1) pthread_create(&frame_thread, NULL, (void*) frame_stats, NULL);
    while(true){
        ZmienTryb(true); // Odczytywanie wejścia bez potrzeby wciskania klawisza Enter
        char a = getchar();
        if(a == 'c') new_client();
        else if(a == 'q') return EXIT_SUCCESS;
    }
}

void thread_start(void *val){
    (void)(val);
    client();
}

void barber(){
    while(true){
        if(verbose > 0) printf("Barber is waiting for customers...\n");
        sem_wait(&waiting_clients); // Oczekiwanie na klientów
        // Sprawdzenie czy klienci zajęli swoje miejsca w poczekalni
        pthread_mutex_lock(&waitroom);
        if(verbose > 1) printf("Barber wakes next customer\n");
        clients = queue_dequeue(clients); // Uzyskanie danych klienta
        print_queue(clients, "Clients");
        if(verbose > 1) printf("Barber is doing a haircut\n");
        sem_post(&barber_ready); // Obudzenie klienta
        // Odblokowanie poczekalni aby klient mógł wejść do gabinetu
        pthread_mutex_unlock(&waitroom);
        sem_wait(&haircut_done); // Oczekiwanie na zakończenie strzyżenia
        if(verbose > 1) printf("Barber has done a haircut\n");
    }
}

void new_client(){
    // Każdy klient uzyskuje inną zmienną warunkową
    pthread_cond_t *cond = malloc(sizeof(pthread_cond_t));
    pthread_cond_init(cond, NULL);
    pthread_create(malloc(sizeof(pthread_t)), NULL, (void*) thread_start, (void*) cond);
}

void client(){
    // Klient wchodzi do poczekalni
    pthread_mutex_lock(&waitroom);
    // Klient bierze numerek (id)
    int id = take_number();
    if(verbose > 0) printf("New client takes number: %d\n", id);
    // Klient próbuje usiąść
    if(sem_trywait(&waitroom_seats) != 0){
        // Klient rezygnuje
        if(verbose > 0) printf("There's no free seats! Client %d goes out\n", id);
        resign_count++;
        // Zwiększenie licznika i dodanie do kolejki zrezygnowanych
        queue_enqueue(resigned, id);
        print_queue(resigned, "Resigned");
        print_stats();
        pthread_mutex_unlock(&waitroom);
        return;
    }
    // Dodanie klienta do kolejki
    queue_enqueue(clients, id);
    print_queue(clients, "Clients");
    // Zwiększenie liczby oczekujących klientów
    sem_post(&waiting_clients);
    print_stats();
    if(verbose > 1) printf("Client %d waits for haircut\n", id);
    pthread_mutex_unlock(&waitroom);
    // Klient czeka na swoją kolej
    sem_wait(&barber_ready);
    // Klient zwalnia miejsce w poczekalni
    sem_post(&waitroom_seats);
    // Klient opuszcza poczekalnie

    // Wejście do gabinetu
    pthread_mutex_lock(&haircut);
    // Ustawienie id aktualnie obsługiwanego klienta
    served_client = id;
    print_stats();
    if(verbose > 0) printf("Client %d is getting a haircut\n", id);
    usleep(1000*haircut_time);
    // Zakończenie strzyżenia
    sem_post(&haircut_done);
    served_client = -1;
    print_stats();
    // Opuszczenie gabinetu
    pthread_mutex_unlock(&haircut);
}

int take_number(){
    pthread_mutex_lock(&number);
    client_count++;
    pthread_mutex_unlock(&number);
    return client_count;
}

void frame_stats(){
    while(true){
        int waiting, x = 80, y = 0;
        sem_getvalue(&waiting_clients, &waiting);

        int served_len = 20, waiting_len = 23, resign_len = 21;

        if(served_client == -1) served_len = 0;
        else served_len += digit_count(served_client);

        waiting_len += digit_count(waiting) + digit_count(waitroom_limit);
        resign_len += digit_count(resign_count);

        int len = bigger(served_len, bigger(waiting_len, resign_len));
        x -= len;

        UstawKursor(x, y++);
        printf(" +");
        for(int i=0; i<len-3; i++) printf("-");
        printf("+");

        if(served_client != -1){
            UstawKursor(x, y++);
            printf(" | Served client: %*s%d |", len-served_len, "", served_client);
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

void print_stats(){
    if(debug != 1) return;
    int waiting;
    sem_getvalue(&waiting_clients, &waiting);
    printf("Res: %d WRomm: %d/%d ", resign_count, waiting, waitroom_limit);
    if(served_client != -1) printf("[in: %d]", served_client);
    printf("\n");
}

void parse_args(int argc, char *argv[]){
    for(int i=1; i<argc; i++){
        if(strcmp(argv[i], "--debug") == 0) debug = 1;
        else if(strcmp(argv[i], "--debug-experimental") == 0) debug = 2;
        else if(strcmp(argv[i], "--no-verbose") == 0) verbose = 0;
        else if(strcmp(argv[i], "--very-verbose") == 0) verbose = 2;
        else if(strcmp(argv[i], "--haircut-time") == 0){
            i++;
            int a = parse_int(argv[i]);
            if(a == -1){
                printf("Invalid haircut time!\n");
                exit(EXIT_FAILURE);
            }
            else haircut_time = a;
        }
        else if(strcmp(argv[i], "--waitroom-size") == 0){
            i++;
            int a = parse_int(argv[i]);
            if(a == -1){
                printf("Invalid waitroom size!\n");
                exit(EXIT_FAILURE);
            }
            else waitroom_limit = a;
        }
        else {
            printf("Ivalid option \"%s\"!\n", argv[i]);
            exit(EXIT_FAILURE);
        }
    }
}

int parse_int(const char *number){
    int n = 0;
    for(int i=0; i<(int)strlen(number); i++){
        if(number[i] >=48 && number[i] <= 57){ // Sprawdza czy znak jest cyfrą
            n *= 10;
            n += (number[i]-48); // Dopisanie cyfry na koniec liczby
        }
        else return -1; // Błąd, ciąg zawiera niedozwolone znaki
    }
    return n;
}


void print_queue(thread_queue *queue, const char *name){
    if(debug < 1) return;
    printf("%s queue: { ", name);
    while(queue->next != NULL){
        queue = queue->next;
        printf("%d, ", queue->id);
    }
    printf("}\n");
}

