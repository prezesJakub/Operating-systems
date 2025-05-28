#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

#define MAX_MEDICINE 6
#define MEDICINE_PER_CONSULT 3
#define MAX_WAITING_PATIENTS 3

int waiting_patients = 0;
int medicine_stock = 0;
int remaining_patients = 0;
bool pharmacist_waiting = false;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t doctor_wakeup = PTHREAD_COND_INITIALIZER;
pthread_cond_t medicine_available = PTHREAD_COND_INITIALIZER;
pthread_cond_t space_in_waiting_room = PTHREAD_COND_INITIALIZER;
pthread_cond_t pharmacist_done = PTHREAD_COND_INITIALIZER;

int patient_queue[MAX_WAITING_PATIENTS];
int queue_pos = 0; 

void log_time() {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char buf[26];
    strftime(buf, 26, "%H:%M:%S", tm_info);
    printf("[%s] - ", buf);
    fflush(stdout);
}

void *patient_thread(void *arg) {
    int id = *(int *)arg;
    free(arg);

    while(true) {
        int travel_time = rand() % 4 + 2;
        log_time(); printf("Pacjent(%d): Ide do szpitala, bede za %d s\n", id, travel_time);
        sleep(travel_time);

        pthread_mutex_lock(&mutex);
        if(remaining_patients <= 0) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        if(waiting_patients >= MAX_WAITING_PATIENTS) {
            int wait = rand() % 3 + 1;
            log_time(); printf("Pacjent(%d): Za dużo pacjentów, wracam później za %d s\n", id, wait);
            pthread_mutex_unlock(&mutex);
            sleep(wait);
            continue;
        }

        waiting_patients++;
        patient_queue[queue_pos++] = id;
        log_time(); printf("Pacjent(%d): Czeka %d pacjentów na lekarza\n", id, waiting_patients);

        if(waiting_patients == MAX_WAITING_PATIENTS && medicine_stock >= MEDICINE_PER_CONSULT) {
            log_time(); printf("Pacjent(%d): budzę lekarza\n", id);
            pthread_cond_signal(&doctor_wakeup);
        }

        pthread_mutex_unlock(&mutex);
        break;
    }
    pthread_exit(NULL);
}

void *pharmacist_thread(void *arg) {
    int id = *(int *)arg;
    free(arg);

    int travel_time = rand() % 11 + 5;
    log_time(); printf("Farmaceuta(%d): Idę do szpitala, będę za %d s\n", id, travel_time);
    sleep(travel_time);

    pthread_mutex_lock(&mutex);

    while(medicine_stock >= MAX_MEDICINE) {
        log_time(); printf("Farmaceuta(%d): Czekam na opróżnienie apteczki\n", id);
        pthread_cond_wait(&medicine_available, &mutex);
    }

    pharmacist_waiting = true;

    if(medicine_stock < MEDICINE_PER_CONSULT) {
        log_time(); printf("Farmaceuta(%d): Budzę lekarza\n", id);
        pthread_cond_signal(&doctor_wakeup);
    }

    pthread_cond_wait(&pharmacist_done, &mutex);
    pharmacist_waiting = false;

    pthread_mutex_unlock(&mutex);

    log_time(); printf("Farmaceuta(%d): Zakończyłem dostawę\n", id);

    pthread_exit(NULL);
}

void *doctor_thread(void *arg) {
    while(1) {
        pthread_mutex_lock(&mutex);

        while(!(
            (waiting_patients == MAX_WAITING_PATIENTS && medicine_stock >= MEDICINE_PER_CONSULT)
            || (pharmacist_waiting && medicine_stock < MEDICINE_PER_CONSULT)
        )) {
            log_time(); printf("Lekarz: zasypiam\n");
            pthread_cond_wait(&doctor_wakeup, &mutex);
            log_time(); printf("Lekarz: budzę się\n");
        }

        if(waiting_patients == MAX_WAITING_PATIENTS && medicine_stock >= MEDICINE_PER_CONSULT) {
            medicine_stock -= MEDICINE_PER_CONSULT;
            log_time(); printf("Lekarz: Konsultuję pacjentów %d, %d, %d\n",
                patient_queue[0], patient_queue[1], patient_queue[2]);
            
            queue_pos = 0;

            pthread_mutex_unlock(&mutex);
            sleep(rand() % 3 + 2);
            pthread_mutex_lock(&mutex);

            remaining_patients -= 3;
            waiting_patients = 0;
            pthread_cond_broadcast(&space_in_waiting_room);
            log_time(); printf("Pacjent(...): Kończę wizytę\n");

            pthread_cond_signal(&medicine_available);
        }
        else if(pharmacist_waiting && medicine_stock < MAX_MEDICINE) {
            log_time(); printf("Lekarz: przyjmuję dostawę leków\n");
            int delivery = MAX_MEDICINE - medicine_stock;
            medicine_stock += delivery;
            sleep(rand() % 3 + 1);
            pthread_cond_signal(&pharmacist_done);
        }

        pthread_mutex_unlock(&mutex);

        if(remaining_patients <= 0) break;
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Użycie: %s <liczba_pacjentów> <liczba_farmaceutów>\n", argv[0]);
        return 1;
    }

    int num_patients = atoi(argv[1]);
    int num_pharmacists = atoi(argv[2]);
    remaining_patients = num_patients;

    srand(time(NULL));

    pthread_t doctor;
    pthread_create(&doctor, NULL, doctor_thread, NULL);

    pthread_t patients[num_patients];
    pthread_t pharmacists[num_pharmacists];

    for(int i=0; i < num_patients; i++) {
        int *id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&patients[i], NULL, patient_thread, id);
    }

    for(int i=0; i < num_pharmacists; i++) {
        int *id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&pharmacists[i], NULL, pharmacist_thread, id);
    }

    for(int i=0; i < num_patients; i++) {
        pthread_join(patients[i], NULL);
    }

    pthread_join(doctor, NULL);

    for(int i=0; i < num_pharmacists; i++) {
        pthread_join(pharmacists[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&doctor_wakeup);
    pthread_cond_destroy(&medicine_available);
    pthread_cond_destroy(&space_in_waiting_room);
    pthread_cond_destroy(&pharmacist_done);

    printf("Szpital zakończył pracę.\n");
    return 0;
}