#include <stdio.h>
#include <stdlib.h>
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
    struct sembuf sb = { semnum, op, 0 };
    semop(semid, &sb, 1);
}

int main() {
    int shmid = shmget(SHM_KEY, sizeof(print_queue), 0666);
    print_queue *queue = (print_queue *)shmat(shmid, NULL, 0);
    int semid = semget(SEM_KEY, 3, 0666);

    while(1) {
        sem_op(semid, 2, -1);
        sem_op(semid, 0, -1);

        char job[JOB_LEN];
        strncpy(job, queue->jobs[queue->head], JOB_LEN - 1);
        job[JOB_LEN-1] = '\0';
        queue->head = (queue->head + 1)%QUEUE_SIZE;
        queue->count--;

        sem_op(semid, 0, 1);
        sem_op(semid, 1, 1);

        printf("Drukowanie: ");
        for(int i=0; i<JOB_LEN - 1; i++) {
            printf("%c", job[i]);
            fflush(stdout);
            sleep(1);
        }
        printf("\n");
    }
    return 0;
}