#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

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

int main() {
    int server_queue = msgget(SERVER_KEY, 0);
    if(server_queue == -1) {
        perror("msgget (server)");
        exit(1);
    }

    key_t client_key = ftok(".", getpid());
    int client_queue = msgget(client_key, IPC_CREAT | 0666);
    if(client_queue == -1) {
        perror("msgget (client)");
        exit(1);
    }

    message init_msg;
    init_msg.mtype = MSG_INIT;
    init_msg.client_id = 0;
    init_msg.queue_key = client_key;

    if(msgsnd(server_queue, &init_msg, sizeof(message) - sizeof(long), 0) == -1) {
        perror("msgsnd (init)");
        exit(1);
    }

    message response;
    if(msgrcv(client_queue, &response, sizeof(message) - sizeof(long), MSG_INIT, 0) == -1) {
        perror("msgrcv (init response)");
        exit(1);
    }

    int client_id = response.client_id;
    printf("Połączono z serwerem jako klient #%d\n", client_id);

    pid_t child = fork();

    if(child == 0) {
        message msg;
        while(1) {
            if(msgrcv(client_queue, &msg, sizeof(message) - sizeof(long), MSG_TEXT, 0) != -1) {
                printf("📩 [%d] %s", msg.client_id, msg.text);
            }
        }
    } else {
        char buffer[MAX_MSG_LEN];
        while(fgets(buffer, MAX_MSG_LEN, stdin) != NULL) {
            message msg;
            msg.mtype = MSG_TEXT;
            msg.client_id = client_id;
            strncpy(msg.text, buffer, MAX_MSG_LEN);
            if(msgsnd(server_queue, &msg, sizeof(message) - sizeof(long), 0) == -1) {
                perror("msgsnd (text)");
            }
        }
    }

    return 0;
}