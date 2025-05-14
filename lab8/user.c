#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>

#define SHM_KEY 0x1234
#define SEM_KEY 0x5678
#define QUEUE_SIZE 10
#define JOB_LEN 10

typedef struct {
    char jobs[QUEUE_SIZE][JOB_LEN];
    int head;
    int tail;
    int count;
} print_queue;

void sem_op(int semid, int semnum, int op) {
    struct sembuf sb = {semnum, op, 0};
    semop(semid, &sb, 1);
}

void generate_job(char *buf) {
    for(int i = 0; i < JOB_LEN-1; i++) {
        buf[i] = 'a' + rand() % 26;
    }
    buf[JOB_LEN - 1] = '\0';
}

int main() {
    srand(time(NULL) ^ getpid());

    int shmid = shmget(SHM_KEY, sizeof(print_queue), 0666);
    print_queue *queue = (print_queue *)shmat(shmid, NULL, 0);
    int semid = semget(SEM_KEY, 3, 0666);

    while(1) {
        char job[JOB_LEN];
        generate_job(job);

        sem_op(semid, 1, -1);
        sem_op(semid, 0, -1);

        strncpy(queue->jobs[queue->tail], job, JOB_LEN);
        queue->tail = (queue->tail + 1) % QUEUE_SIZE;
        queue->count++;

        printf("Dodano zadanie: %s\n", job);

        sem_op(semid, 0, 1);
        sem_op(semid, 2, 1);

        sleep(rand() % 5 + 1);
    }

    return 0;
}