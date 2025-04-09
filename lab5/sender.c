#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

volatile sig_atomic_t received_ack = 0;

void sigusr1_handler(int sig) {
    received_ack = 1;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Użycie: %s <PID_catcher> <tryb>\n", argv[0]);
        exit(1);
    }

    pid_t pid_catcher = atoi(argv[1]);
    int mode = atoi(argv[2]);

    struct sigaction sa;
    sa.sa_handler = sigusr1_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);

    union sigval value;
    value.sival_int = mode;

    if (sigqueue(pid_catcher, SIGUSR1, value) == -1) {
        perror("Błąd podczas wysyłania sygnału");
        exit(1);
    }

    printf("Wysłano tryb %d do catcher'a\n", mode);

    while (!received_ack) {
        pause();
    }

    printf("Otrzymano potwierdzenie od catcher'a\n");
    return 0;
}