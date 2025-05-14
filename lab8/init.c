#include <stdio.h>
#include <stdlib.h>
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

void sem_set(int semid, int semnum, int value) {
    union semun {
        int val;
    } arg;
    arg.val = value;
    semctl(semid, semnum, SETVAL, arg);
}

int main() {
    int shmid = shmget(SHM_KEY, sizeof(print_queue), IPC_CREAT | 0666);
    if(shmid == -1) {
        perror("shmget");
        exit(1);
    }

    print_queue *queue = (print_queue *)shmat(shmid, NULL, 0);
    memset(queue, 0, sizeof(print_queue));
    shmdt(queue);

    int semid = semget(SEM_KEY, 3, IPC_CREAT | 0666);
    if(semid == -1) {
        perror("semget");
        exit(1);
    }

    sem_set(semid, 0, 1);
    sem_set(semid, 1, QUEUE_SIZE);
    sem_set(semid, 2, 0);

    printf("Inicjalizacja zakończona\n");
    return 0;
}