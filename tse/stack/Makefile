TOPDIR = $(shell cd ..; pwd)
include $(TOPDIR)/Rules.make

stack.o: stack.c stack.h
	$(CC) $(CFLAGS) stack.c

clean:
	rm -f stack.o
