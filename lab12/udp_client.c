#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>

#define LENGTH 2048

volatile sig_atomic_t flag = 0;
int sockfd;
struct sockaddr_in server_addr;
socklen_t server_len;

char name[32];

void str_trim_lf(char *arr, int length) {
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

void *send_msg_handler(void *arg) {
    char message[LENGTH];
    while(1) {
        if (flag) break;
        printf("> ");
        fgets(message, LENGTH, stdin);
        str_trim_lf(message, LENGTH);
        sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&server_addr, server_len);
        if(strcmp(message, "STOP") == 0) {
            break;
        }
    }
    return NULL;
}

void *recv_msg_handler(void *arg) {
    char message[LENGTH];
    while(1) {
        int len = recvfrom(sockfd, message, LENGTH, 0, NULL, NULL);
        if(len > 0) {
            message[len] = '\0';
            if(strcmp(message, "ALIVE") == 0) {
                sendto(sockfd, "ALIVE", 5, 0, (struct sockaddr *)&server_addr, server_len);
            } else {
                printf("%s", message);
                printf("\n> ");
                fflush(stdout);
            }
        }
    }
    return NULL;
}

int main(int argc, char **argv) {
    if(argc != 4) {
        printf("Usage: %s <name> <ip> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    strcpy(name, argv[1]);
    const char *ip = argv[2];
    int port = atoi(argv[3]);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server_addr.sin_addr);
    server_len = sizeof(server_addr);

    sendto(sockfd, name, strlen(name), 0, (struct sockaddr *)&server_addr, server_len);

    signal(SIGINT, catch_ctrl_c_and_exit);

    pthread_t send_thread, recv_thread;
    pthread_create(&send_thread, NULL, send_msg_handler, NULL);
    pthread_create(&recv_thread, NULL, recv_msg_handler, NULL);

    pthread_join(send_thread, NULL);
    pthread_cancel(recv_thread);

    close(sockfd);
    return 0;
}