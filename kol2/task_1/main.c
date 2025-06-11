#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <time.h>

#ifdef USE_POSIX
#include <semaphore.h>
#include <fcntl.h> // For O_* constants
#endif

#define FILE_NAME "common.txt"
#define SEM_NAME "/kol_sem" // For POSIX semaphore

int main(int argc, char** args) {
    if(argc != 4) {
        printf("Not a suitable number of program parameters\n");
        return 1;
    }

    int fd = open(FILE_NAME, O_WRONLY | O_CREAT | O_TRUNC , 0644);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    int parentLoopCounter = atoi(args[1]);
    int childLoopCounter = atoi(args[2]);
    int max_sleep_time = atoi(args[3]);

    char buf[128];
    pid_t childPid;

#ifdef USE_POSIX
    sem_t *sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0644, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        return 1;
    }
#else
    key_t key = ftok("/tmp", 'K');
    if (key == -1) {
        perror("ftok");
        return 1;
    }

    int semid = semget(key, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget");
        return 1;
    }

    if (semctl(semid, 0, SETVAL, 1) == -1) {
        perror("semctl");
        return 1;
    }
    // STUDENT TASK: Create and initialize a System V semaphore with initial value 1
#endif

    if ((childPid = fork())) {
        int status = 0;
        srand((unsigned)time(0) ^ getpid());

        while (parentLoopCounter--) {
            int s = rand() % max_sleep_time + 1;
            sleep(s);

#ifdef USE_POSIX
            sem_wait(sem);
#else
            struct sembuf op = {0, -1, 0};
            semop(semid, &op, 1);
#endif
            // STUDENT TASK: Enter critical section using the selected semaphore

            snprintf(buf, sizeof(buf), "Wpis rodzica. Petla %d. Spalem %d\n", parentLoopCounter, s);
            write(fd, buf, strlen(buf));
            write(1, buf, strlen(buf));

            // STUDENT TASK: Exit critical section using the selected semaphore
#ifdef USE_POSIX
            sem_post(sem);
#else
            struct sembuf op2 = {0, 1, 0};
            semop(semid, &op2, 1);
#endif
        }

        waitpid(childPid, &status, 0);

#ifdef USE_POSIX
        sem_close(sem);
        sem_unlink(SEM_NAME);
#else
        semctl(semid, 0, IPC_RMID);
#endif

    } else {
        srand((unsigned)time(0) ^ getpid());

        while (childLoopCounter--) {
            int s = rand() % max_sleep_time + 1;
            sleep(s);

            // STUDENT TASK: Enter critical section using the selected semaphore

#ifdef USE_POSIX
            sem_wait(sem);
#else
            struct sembuf op = {0, -1, 0};
            semop(semid, &op, 1);
#endif

            snprintf(buf, sizeof(buf), "Wpis dziecka. Petla %d. Spalem %d\n", childLoopCounter, s);
            write(fd, buf, strlen(buf));
            write(1, buf, strlen(buf));

            // STUDENT TASK: Exit critical section using the selected semaphore

#ifdef USE_POSIX
            sem_post(sem);
#else
            struct sembuf op2 = {0, 1, 0};
            semop(semid, &op2, 1);
#endif
        }

        close(fd);
        _exit(0);
    }

    close(fd);
    return 0;
}