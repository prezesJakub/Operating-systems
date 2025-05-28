#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define NAME_LEN 32

typedef struct {
    struct sockaddr_in addr;
    int sockfd;
    char name[NAME_LEN];
    time_t last_alive;
    int active;
    pthread_t thread_id;
} client_t;

client_t *clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void str_trim_lf(char *arr, int length) {
    for(int i=0; i < length; i++) {
        if(arr[i] == '\n') {
            arr[i] = '\0';
            break;
        }
    }
}

void add_client(client_t *cl) {
    pthread_mutex_lock(&clients_mutex);
    for(int i=0; i<MAX_CLIENTS; i++) {
        if(!clients[i]) {
            clients[i] = cl;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void remove_client(int sockfd) {
    pthread_mutex_lock(&clients_mutex);
    for(int i=0; i<MAX_CLIENTS; i++) {
        if(clients[i]) {
            if(clients[i]->sockfd == sockfd) {
                close(clients[i]->sockfd);
                free(clients[i]);
                clients[i] = NULL;
                break;
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void send_message(char *s, int sockfd) {
    pthread_mutex_lock(&clients_mutex);
    for(int i=0; i<MAX_CLIENTS; i++) {
        if(clients[i]) {
            if(clients[i]->sockfd != sockfd) {
                if(write(clients[i]->sockfd, s, strlen(s)) < 0) {
                    perror("write to descriptor failed");
                    break;
                }
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void send_private_message(char *s, char *recipient) {
    pthread_mutex_lock(&clients_mutex);
    for(int i=0; i<MAX_CLIENTS; i++) {
        if(clients[i]) {
            if(strcmp(clients[i]->name, recipient) == 0) {
                if(write(clients[i]->sockfd, s, strlen(s)) < 0) {
                    perror("write to descriptor failed");
                }
                break;
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void send_list(int sockfd) {
    char s[BUFFER_SIZE];
    s[0] = '\0';
    strcat(s, "Active clients:\n");
    pthread_mutex_lock(&clients_mutex);
    for(int i=0; i<MAX_CLIENTS; i++) {
        if(clients[i]) {
            strcat(s, clients[i]->name);
            strcat(s, "\n");
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    write(sockfd, s, strlen(s));
}

void *handle_client(void *arg) {
    char buff_out[BUFFER_SIZE];
    char name[NAME_LEN];
    int leave_flag = 0;
    client_t *cli = (client_t *)arg;

    if(recv(cli->sockfd, name, NAME_LEN, 0) <= 0 || strlen(name) < 2 || strlen(name) >= NAME_LEN-1) {
        printf("Didn't enter the name.\n");
        leave_flag = 1;
    } else {
        strcpy(cli->name, name);
        sprintf(buff_out, "%s has joined\n", cli->name);
        printf("%s", buff_out);
        send_message(buff_out, cli->sockfd);
    }

    add_client(cli);

    while(1) {
        if(leave_flag || cli->active == 0) {
            break;
        }

        int receive = recv(cli->sockfd, buff_out, BUFFER_SIZE, 0);
        if(receive > 0) {
            buff_out[receive] = '\0';
            str_trim_lf(buff_out, strlen(buff_out));

            if(strcmp(buff_out, "STOP") == 0) {
                sprintf(buff_out, "%s has left\n", cli->name);
                printf("%s", buff_out);
                send_message(buff_out, cli->sockfd);
                leave_flag = 1;
            } else if(strcmp(buff_out, "LIST") == 0) {
                send_list(cli->sockfd);
            } else if(strncmp(buff_out, "2ALL ", 5) == 0) {
                char message[BUFFER_SIZE];
                sprintf(message, "[%s]: %s\n", cli->name, buff_out + 5);
                send_message(message, cli->sockfd);
            } else if(strncmp(buff_out, "2ONE ", 5) == 0) {
                char *recipient = strtok(buff_out + 5, " ");
                char *message_body = strtok(NULL, "\0");
                if(recipient && message_body) {
                    char message[BUFFER_SIZE];
                    sprintf(message, "[%s to %s]: %s\n", cli->name, recipient, message_body);
                    send_private_message(message, recipient);
                }
            } else if(strcmp(buff_out, "ALIVE") == 0) {
                cli->last_alive = time(NULL);
            } else {
                printf("Unknown command from %s: %s\n", cli->name, buff_out);
            }
        } else if(receive == 0 || strcmp(buff_out, "STOP") == 0) {
            sprintf(buff_out, "%s has left\n", cli->name);
            printf("%s", buff_out);
            send_message(buff_out, cli->sockfd);
            leave_flag = 1;
        } else {
            perror("recv");
            leave_flag = 1;
        }

        bzero(buff_out, BUFFER_SIZE);
    }

    close(cli->sockfd);
    remove_client(cli->sockfd);
    pthread_detach(pthread_self());

    return NULL;
}

void *ping_clients(void *arg) {
    while(1) {
        sleep(60);
        time_t now = time(NULL);
        pthread_mutex_lock(&clients_mutex);
        for(int i=0; i<MAX_CLIENTS; i++) {
            if(clients[i]) {
                if(difftime(now, clients[i]->last_alive) > 90) {
                    printf("Client %s timed out.\n", clients[i]->name);
                    clients[i]->active = 0;
                } else {
                    if(write(clients[i]->sockfd, "ALIVE\n", 6) < 0) {
                        perror("write to descriptor failed");
                    }
                }
            }
        }
        pthread_mutex_unlock(&clients_mutex);
    }
    return NULL;
}

int main(int argc, char **argv) {
    if(argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const int port = atoi(argv[1]);

    int option = 1;
    int listenfd = 0, connfd = 0;

    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;
    pthread_t tid;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd < 0) {
        perror("Socket failed");
        return EXIT_FAILURE;
    }

    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Bind failed");
        return EXIT_FAILURE;
    }

    if(listen(listenfd, 10) < 0) {
        perror("Listen failed");
        return EXIT_FAILURE;
    }

    printf("=== WELCOME TO THE CHATROOM ===\n");

    pthread_t ping_thread;
    pthread_create(&ping_thread, NULL, &ping_clients, NULL);

    while(1) {
        socklen_t clilen = sizeof(cli_addr);
        connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);

        if((connfd) < 0) {
            perror("Accept failed");
            continue;
        }

        client_t *cli = (client_t *)malloc(sizeof(client_t));
        cli->addr = cli_addr;
        cli->sockfd = connfd;
        cli->last_alive = time(NULL);
        cli->active = 1;
        
        pthread_create(&tid, NULL, &handle_client, (void*)cli);
    }

    return EXIT_SUCCESS;
}