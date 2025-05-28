#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>

#define LENGTH 2048

volatile sig_atomic_t flag = 0;
int sockfd = 0;
char name[32];

void str_overwrite_stdout() {
    printf("\r%s", "> ");
    fflush(stdout);
}

void str_trim_lf(char* arr, int length) {
    for(int i=0; i<length; i++) {
        if(arr[i] == '\n') {
            arr[i] = '\0';
            break;
        }
    }
}

void catch_ctrl_c_and_exit(int sig) {
    flag = 1;
}

void send_msg_handler() {
    char message[LENGTH] = {};
    char buffer[LENGTH + 32] = {};

    while(1) {
        str_overwrite_stdout();
        fgets(message, LENGTH, stdin);
        str_trim_lf(message, LENGTH);

        if(strcmp(message, "STOP") == 0) {
            send(sockfd, message, strlen(message), 0);
            break;
        } else {
            send(sockfd, message, strlen(message), 0);
        }

        bzero(message, LENGTH);
        bzero(buffer, LENGTH + 32);
    }
    catch_ctrl_c_and_exit(2);
}

void recv_msg_handler() {
    char message[LENGTH] = {};
    while (1) {
        int receive = recv(sockfd, message, LENGTH, 0);
        if (receive > 0) {
            if (strncmp(message, "ALIVE", 5) == 0) {
                send(sockfd, "ALIVE", 5, 0);
            } else {
                printf("%s", message);
                str_overwrite_stdout();
            }
        } else if (receive == 0) {
            break;
        }
        bzero(message, LENGTH);
    }
}

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <name> <ip> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    strcpy(name, argv[1]);
    const char *ip = argv[2];
    int port = atoi(argv[3]);

    signal(SIGINT, catch_ctrl_c_and_exit);

    struct sockaddr_in server_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        return EXIT_FAILURE;
    }

    send(sockfd, name, strlen(name), 0);

    pthread_t send_thread;
    if (pthread_create(&send_thread, NULL, (void *)send_msg_handler, NULL) != 0) {
        perror("pthread_create");
        return EXIT_FAILURE;
    }

    pthread_t recv_thread;
    if (pthread_create(&recv_thread, NULL, (void *)recv_msg_handler, NULL) != 0) {
        perror("pthread_create");
        return EXIT_FAILURE;
    }

    while (1) {
        if (flag) {
            send(sockfd, "STOP", 4, 0);
            break;
        }
    }

    pthread_join(send_thread, NULL);
    pthread_join(recv_thread, NULL);

    close(sockfd);

    return EXIT_SUCCESS;
}