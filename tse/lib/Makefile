TOPDIR = $(shell cd ..; pwd)
include $(TOPDIR)/Rules.make

all: stack.o misc.o

stack.o: stack.c stack.h
	$(CC) $(CFLAGS) stack.c

misc.o: misc.c misc.h
	$(CC) $(CFLAGS) misc.c

clean:
	rm -f *.o
