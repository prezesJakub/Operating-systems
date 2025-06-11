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
    void *shmAddr = NULL;
    char dataAddr[] = "Hello";

    // Create a unique key to identify the current System V inter-process communication (IPC)
    // TASK: complete the function to generate IPC key
    key_t key = ftok(PATH, ID);
    if (key == -1) {
        perror("ftok");
        exit(1);
    }

    // Create a shared memory segment
    // TASK: complete shmget() call
    int shmid = shmget(key, BUFFER_SIZE, IPC_CREAT | 0666);
    if (shmid == -1)
    {
        fprintf(stderr, "shmget error : %s\n", strerror(errno));
        exit(1);
    }

    // Map shared memory segment to process address space
    // TASK: complete shmat() call
    shmAddr = shmat(shmid, NULL, 0);
    if (shmAddr == (void *) -1 )
    {
        fprintf(stderr, "Failed to attach shared memory: %s\n", strerror(errno));
        exit(1);
    }

    // Copy dataAddr to shmAddr
    // TASK: complete memory copy
    strncpy((char *)shmAddr, dataAddr, BUFFER_SIZE);

    // TASK: detach process memory from shared memory
    if ( shmdt(shmAddr) == -1)
    {
        fprintf(stderr, "Failed to detach: %s\n", strerror(errno));
    }

    return 0;
}

/*
gcc -o shm_write shm_write_student.c
*/