#define MAX_CLIENTS 10
#define MAX_ID_LEN 32
#define MAX_MSG_LEN 512

typedef enum {
    MSG_LIST,
    MSG_TO_ALL,
    MSG_TO_ONE,
    MSG_STOP,
    MSG_ALIVE,
    MSG_REGISTER,
    MSG_INCOMING
} MessageType;

typedef struct {
    MessageType type;
    char sender[MAX_ID_LEN];
    char receiver[MAX_ID_LEN];
    char message[MAX_MSG_LEN];
} Message;