#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define NAME_LEN 32

typedef struct {
    struct sockaddr_in addr;
    char name[NAME_LEN];
    time_t last_alive;
    int active;
} client_t;

client_t clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

int server_socket;

void str_trim_lf(char *arr, int length) {
    for(int i=0; i<length; i++) {
        if(arr[i] == '\n') {
            arr[i] = '\0';
            break;
        }
    }
}

int find_client(struct sockaddr_in *addr) {
    for(int i=0; i<client_count; i++) {
        if(clients[i].addr.sin_addr.s_addr == addr->sin_addr.s_addr &&
            clients[i].addr.sin_port == addr->sin_port) {
                return i;
        }
    }
    return -1;
}

void add_client(struct sockaddr_in *addr, const char *name) {
    if(client_count >= MAX_CLIENTS) return;
    client_t *cli = &clients[client_count++];
    cli->addr = *addr;
    strncpy(cli->name, name, NAME_LEN);
    cli->last_alive = time(NULL);
    cli->active = 1;
}

void remove_client(int index) {
    clients[index] = clients[--client_count];
}

void send_message_to_all(const char *msg, struct sockaddr_in *exclude_addr) {
    for(int i=0; i<client_count; i++) {
        if(memcmp(&clients[i].addr, exclude_addr, sizeof(struct sockaddr_in)) != 0) {
            sendto(server_socket, msg, strlen(msg), 0, (struct sockaddr *)&clients[i].addr, sizeof(clients[i].addr));
        }
    }
}

void send_private(const char *recipient, const char *msg) {
    for(int i=0; i<client_count; i++) {
        if(strcmp(clients[i].name, recipient) == 0) {
            sendto(server_socket, msg, strlen(msg), 0, (struct sockaddr *)&clients[i].addr, sizeof(clients[i].addr));
            break;
        }
    }
}

void send_list(struct sockaddr_in *addr) {
    char buffer[BUFFER_SIZE] = "Active clients:\n";
    for(int i=0; i<client_count; i++) {
        strcat(buffer, clients[i].name);
        strcat(buffer, "\n");
    }
    sendto(server_socket, buffer, strlen(buffer), 0, (struct sockaddr *)addr, sizeof(*addr));
}

void *ping_clients(void *arg) {
    while (1) {
        sleep(60);
        time_t now = time(NULL);
        pthread_mutex_lock(&clients_mutex);
        for(int i=0; i<client_count; i++) {
            if(difftime(now, clients[i].last_alive) > 90) {
                printf("Client %s timed out.\n", clients[i].name);
                clients[i].active = 0;
                remove_client(i);
                i--;
            } else {
                sendto(server_socket, "ALIVE", 5, 0, (struct sockaddr *)&clients[i].addr, sizeof(clients[i].addr));
            }
        }
        pthread_mutex_unlock(&clients_mutex);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int port = atoi(argv[1]);
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    char buffer[BUFFER_SIZE];

    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(server_socket < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        return EXIT_FAILURE;
    }

    pthread_t ping_thread;
    pthread_create(&ping_thread, NULL, &ping_clients, NULL);

    printf("=== UDP CHAT SERVER STARTED ===\n");

    while(1) {
        memset(buffer, 0, BUFFER_SIZE);
        int recv_len = recvfrom(server_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_len);
        if (recv_len <= 0) continue;

        buffer[recv_len] = '\0';
        str_trim_lf(buffer, BUFFER_SIZE);

        pthread_mutex_lock(&clients_mutex);
        int index = find_client(&client_addr);

        if(index == -1) {
            add_client(&client_addr, buffer);
            char join_msg[BUFFER_SIZE];
            snprintf(join_msg, sizeof(join_msg), "%.32s has joined.\n", buffer);
            send_message_to_all(join_msg, &client_addr);
            pthread_mutex_unlock(&clients_mutex);
            continue;
        }

        clients[index].last_alive = time(NULL);
        if(strcmp(buffer, "STOP") == 0) {
            char leave_msg[BUFFER_SIZE];
            snprintf(leave_msg, sizeof(leave_msg), "%.32s has left.\n", clients[index].name);
            send_message_to_all(leave_msg, &client_addr);
            remove_client(index);
        } else if (strcmp(buffer, "LIST") == 0) {
            send_list(&client_addr);
        } else if (strncmp(buffer, "2ALL ", 5) == 0) {
            char message[BUFFER_SIZE];
            snprintf(message, sizeof(message), "[%.32s]: %.900s\n", clients[index].name, buffer + 5);
            send_message_to_all(message, &client_addr);
        } else if (strncmp(buffer, "2ONE ", 5) == 0) {
            char copy[BUFFER_SIZE];
            strncpy(copy, buffer + 5, BUFFER_SIZE - 1);
            copy[BUFFER_SIZE - 1] = '\0';

            char *recipient = strtok(copy, " ");
            char *message_body = strtok(NULL, "\0");

            if (recipient && message_body) {
                char message[BUFFER_SIZE];
                snprintf(message, sizeof(message), "[%.32s to %.32s]: %.900s\n", clients[index].name, recipient, message_body);
                send_private(recipient, message);
            }
        } else if (strcmp(buffer, "ALIVE") == 0) {

        } else {
            char unknown_msg[BUFFER_SIZE];
            snprintf(unknown_msg, sizeof(unknown_msg), "Unknown command: %.1000s\n", buffer);
            sendto(server_socket, unknown_msg, strlen(unknown_msg), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
        }
        
        pthread_mutex_unlock(&clients_mutex);
    }

    close(server_socket);
    return 0;
}