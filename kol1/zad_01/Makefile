CC = gcc
CFLAGS = -Wall -fPIC
LDFLAGS = -shared

all: lib/libbibl1.so bin/zad1

lib/libbibl1.so: bibl1.c bibl1.h
	$(CC) $(CFLAGS) -o lib/bibl1.o -c bibl1.c
	$(CC) $(LDFLAGS) -o lib/libbibl1.so lib/bibl1.o

bin/zad1: zad1.c
	$(CC) -o bin/zad1 zad1.c -ldl

test: all
	@echo "xxxxxx"
	LD_LIBRARY_PATH=./lib ./bin/zad1

clean:
	rm -f lib/*.o lib/*.so bin/zad1