CC = gcc

CFLAGS = -Wall -g

all: countdown

countdown: countdown.c
	$(CC) $(CFLAGS) countdown.c -o countdown

clean:
	rm -f countdown

.PHONY: all countdown clean