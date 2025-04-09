#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <asm/signal.h>

void sigusr1_handler(int sig) {
    printf("Odebrano sygnał SIGUSR1!\n");
}

int main(int argc, char *argv[]) {
    if(argc != 2) {
        fprintf(stderr, "Użycie: %s <none|ignore|handler|mask>\n", argv[0]);
        return 1;
    }

    char *option = argv[1];

    sigset_t set;

    if(strcmp(option, "none") == 0) {
        printf("Reakcja na SIGUSR1: none\n");
    }
    else if(strcmp(option, "ignore") == 0) {
        signal(SIGUSR1, SIG_IGN);
        printf("Reakcja na SIGUSR1: ignore\n");
    }
    else if(strcmp(option, "handler") == 0) {
        signal(SIGUSR1, sigusr1_handler);
        printf("Reakcja na SIGUSR1: handler\n");
    }
    else if(strcmp(option, "mask") == 0) {
        sigemptyset(&set);
        sigaddset(&set, SIGUSR1);
        sigprocmask(SIG_BLOCK, &set, NULL);
        printf("Reakcja na SIGUSR1: mask\n");
    }
    else {
        fprintf(stderr, "Nieprawidłowy argument: %s. Użyj none, ignore, handler, mask.\n", option);
        return 1;
    }
    raise(SIGUSR1);

    if(strcmp(option, "mask") == 0) {
        sigpending(&set);
        if(sigismember(&set, SIGUSR1)) {
            printf("Sygnał SIGUSR1 oczekuje w kolejce\n");
        }
    }

    pause();
    return 0;
}