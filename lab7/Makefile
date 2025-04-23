CC = gcc
CFLAGS = -Wall -O2
OBJS = server client

all: $(OBJS)

server: server.c
	$(CC) $(CFLAGS) -o server server.c

client: client.c
	$(CC) $(CFLAGS) -o client client.c

clean:
	rm -f server client