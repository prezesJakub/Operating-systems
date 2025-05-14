CC = gcc
CFLAGS = -Wall -O2

all: init user printer

init: init.c
	$(CC) $(CFLAGS) -o init init.c

user: user.c
	$(CC) $(CFLAGS) -o user user.c

printer: printer.c
	$(CC) $(CFLAGS) -o printer printer.c

clean:
	rm -f init user printer