#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

int received_signals = 0;
int mode = 0;
pid_t sender_pid = -1;
pid_t counting_pid = 0;

void sigint_handler(int sig) {
    printf("Wciśnięto CTRL+C\n");
}

void count_numbers() {
    int count = 1;
    while (1) {
        printf("%d\n", count++);
        sleep(1);
    }
}

void sigusr1_handler(int sig, siginfo_t *info, void *context) {
    received_signals++;
    sender_pid = info->si_pid;
    int new_mode = info->si_value.sival_int;

    if (new_mode != 2 && counting_pid > 0) {
        kill(counting_pid, SIGTERM);
        waitpid(counting_pid, NULL, 0);
        counting_pid = 0;
    }

    mode = new_mode;

    if (mode == 1) {
        printf("Liczba otrzymanych sygnałów: %d\n", received_signals);
    } 
    else if (mode == 2) {
        if (counting_pid > 0) {
            kill(counting_pid, SIGTERM);
            waitpid(counting_pid, NULL, 0);
            counting_pid = 0;
        }
        counting_pid = fork();
        if (counting_pid == 0) {
            count_numbers();
            exit(0);
        }
    } 
    else if (mode == 3) {
        signal(SIGINT, SIG_IGN);
        printf("Zignorowano CTRL+C\n");
    } 
    else if (mode == 4) {
        signal(SIGINT, SIG_DFL);
        signal(SIGINT, sigint_handler);
        printf("Przywrócono obsługę CTRL+C\n");
    } 
    else if (mode == 5) {
        printf("Zamykanie catchera...\n");
        if (counting_pid > 0) {
            kill(counting_pid, SIGTERM);
            waitpid(counting_pid, NULL, 0);
        }

        kill(sender_pid, SIGUSR1);
        exit(0);
    }

    kill(sender_pid, SIGUSR1);
}

int main() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = sigusr1_handler;
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &sa, NULL);

    printf("PID catcher: %d\n", getpid());

    while (1) {
        pause();
    }
    return 0;
}