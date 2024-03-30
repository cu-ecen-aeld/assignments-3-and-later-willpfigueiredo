.DEFAULT_GOAL:=writer

writer: writer.o
	$(CROSS_COMPILE)gcc -o writer writer.o

writer.o: writer.c
	$(CROSS_COMPILE)gcc -o writer.o writer.c -c -W -Wall -ansi -pedantic

clean:
	rm -rf *.o *~ writer