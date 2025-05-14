CC = gcc
CFLAGS = -Wall -O2

all: life threaded
	
life: grid.c life.c
	$(CC) $(CFLAGS) grid.c life.c -o life -lncursesw
	
threaded: grid.c life.c
	$(CC) $(CFLAGS) grid.c life.c -o life_threaded -lncursesw -lpthread

clean:
	rm -f life life_threaded
