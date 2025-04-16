CC = gcc
CFLAGS = -Wall -O2
CLIENT = client
SERVER = server

all: $(CLIENT) $(SERVER)

$(CLIENT): client.c
	$(CC) $(CFLAGS) -o $(CLIENT) client.c

$(SERVER): server.c
	$(CC) $(CFLAGS) -o $(SERVER) server.c

clean:
	rm -f $(CLIENT) $(SERVER) fifo_request fifo_response