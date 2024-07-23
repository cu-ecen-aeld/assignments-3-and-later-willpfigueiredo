.DEFAULT_GOAL:=aesdsocket
CC ?= $(CROSS_COMPILE)gcc
all: aesdsocket

aesdsocket: aesdsocket.o
	$(CC)  -o aesdsocket aesdsocket.o -pthread -lrt

aesdsocket.o: aesdsocket.c
	$(CC) -o aesdsocket.o aesdsocket.c -c -std=gnu99 -W -Wall -pedantic , --hash-style=gnu

clean:
	rm -rf *.o *~ aesdsocket 