#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MAX_CLIENTS 10
#define MAX_MSG_LEN 256
#define SERVER_KEY 1234

#define MSG_INIT 1
#define MSG_TEXT 2

typedef struct {
    long mtype;
    int client_id;
    key_t queue_key;
    char text[MAX_MSG_LEN];
} message;

typedef struct {
    int client_id;
    int queue_id;
} client_info;

client_info clients[MAX_CLIENTS];
int client_count = 0;

void broadcast(message *msg) {
    for (int i=0; i<client_count; i++) {
        if(clients[i].client_id != msg->client_id) {
            if(msgsnd(clients[i].queue_id, msg, sizeof(message) - sizeof(long), 0) == -1) {
                perror("msgsnd (broadcast)");
            }
        }
    }
}

int main() {
    int server_queue = msgget(SERVER_KEY, IPC_CREAT | 0666);
    if(server_queue == -1) {
        perror("msgget (server)");
        exit(1);
    }

    printf("Serwer uruchomiony...\n");

    message msg;

    while(1) {
        if(msgrcv(server_queue, &msg, sizeof(message) - sizeof(long), 0, 0) == -1) {
            perror("msgrcv");
            continue;
        }

        if(msg.mtype == MSG_INIT) {
            if(client_count >= MAX_CLIENTS) {
                fprintf(stderr, "Za dużo klientów!\n");
                continue;
            }

            int client_queue = msgget(msg.queue_key, 0);
            if(client_queue == -1) {
                perror("msgget (client queue)");
                continue;
            }

            int new_id = client_count + 1;
            clients[client_count].client_id = new_id;
            clients[client_count].queue_id = client_queue;
            client_count++;

            message reply;
            reply.mtype = MSG_INIT;
            reply.client_id = new_id;
            if(msgsnd(client_queue, &reply, sizeof(message) - sizeof(long), 0) == -1) {
                perror("msgsnd (init reply)");
            }
            printf("Klient #%d dołączył do czatu\n", new_id);
        } else if(msg.mtype == MSG_TEXT) {
            printf("Klient #%d: %s", msg.client_id, msg.text);
            broadcast(&msg);
        }
    }
    return 0;
}