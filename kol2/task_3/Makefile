CC = gcc
CFLAGS = -Wall -pthread

all: main

main: main.o
	$(CC) main.o -o main $(CFLAGS)

main.o: main.c
	$(CC) -c main.c $(CFLAGS)

clean:
	rm -f *.o main