.DEFAULT_GOAL:=writer
CC ?= $(CROSS_COMPILE)gcc
all: writer

writer: writer.o
	$(CC) -o writer writer.o

writer.o: writer.c
	$(CC) -o writer.o writer.c -c -W -Wall -ansi -pedantic

clean:
	rm -rf *.o *~ writer
