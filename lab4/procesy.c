#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Użycie: %s <liczba_procesów>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    if (n <= 0) {
        fprintf(stderr, "Podaj liczbę większą od 0!\n");
        return 1;
    }

    for (int i=0; i<n; i++) {
        pid_t pid = fork();
        if(pid < 0) {
            perror("Błąd fork");
            return 1;
        }
        else if (pid == 0) {
            printf("PID rodzica: %d, Mój PID: %d\n", getppid(), getpid());
            exit(0);
        }
    }

    for (int i=0; i<n; i++) {
        wait(NULL);
    }

    printf("%d\n", n);

    return 0;
}