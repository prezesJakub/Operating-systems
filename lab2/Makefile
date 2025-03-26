CC=gcc
CFLAGS = -Wall -std=c17 -fPIC
LIB_DIR=collatz

all: client_static client_shared client_dynamic

libcollatz.a: $(LIB_DIR)/collatz.o 
	ar rcs libcollatz.a $(LIB_DIR)/collatz.o

libcollatz.so: $(LIB_DIR)/collatz.o
	$(CC) -shared -o libcollatz.so $(LIB_DIR)/collatz.o

$(LIB_DIR)/collatz.o: $(LIB_DIR)/collatz.c $(LIB_DIR)/collatz.h
	$(CC) $(CFLAGS) -c $(LIB_DIR)/collatz.c -o $(LIB_DIR)/collatz.o

client_static: client.c libcollatz.a
	$(CC) client.c -o client_static -L. -lcollatz

client_shared: client.c libcollatz.so
	$(CC) client.c -o client_shared -L. -lcollatz

client_dynamic: client_dynamic.c libcollatz.so
	$(CC) client_dynamic.c -o client_dynamic -ldl

clean:
	rm -f $(LIB_DIR)/*.o *.a *.so client_static client_shared client_dynamic