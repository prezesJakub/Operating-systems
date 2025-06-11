CC = gcc
CFLAGS = -Wall -std=c99

all: shm_write shm_read

shm_write: shm_write.c
	$(CC) $(CFLAGS) -o shm_write shm_write.c

shm_read: shm_read.c
	$(CC) $(CFLAGS) -o shm_read shm_read.c

clean:
	rm -f shm_write shm_read