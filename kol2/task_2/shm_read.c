#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <string.h>
#include <errno.h>

#define PATH "/tmp"
#define BUFFER_SIZE 1024
#define ID 0

int main(int argc, char const *argv[])
{
    char *shmAddr;

    // Convert path and identifier to System V IPC key
    // TASK: complete the key creation
    key_t key = ftok(PATH, ID);
    if (key == -1) {
        perror("ftok");
        exit(1);
    }

    // Access the shared memory segment
    // TASK: complete shmget() call
    int shmid = shmget(key, BUFFER_SIZE, 0666);
    if (shmid == -1)
    {
        fprintf(stderr, "shmget error : %s\n", strerror(errno));
        exit(1);
    }

    // Map shared memory segment to process address space
    // TASK: complete shmat() call
    shmAddr = shmat(shmid, NULL, 0);
    if (shmAddr == (void *) -1)
    {
        fprintf(stderr, "Failed to attach: %s\n", strerror(errno));
        exit(1);
    }

    printf("Odczytana wiadomość: %s\n", shmAddr);

    // TASK: detach shared memory
    if(shmdt(shmAddr) == -1) {
        fprintf(stderr, "Failed to detach: %s\n", strerror(errno));
    }

    // TASK: remove shared memory segment
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        fprintf(stderr, "Failed to remove shared memory: %s\n", strerror(errno));
    }

    return 0;
}

/*
gcc -o shm_read shm_read_student.c
*/