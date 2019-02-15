TOPDIR = $(shell cd ..; pwd)
include $(TOPDIR)/Rules.make

lex.uri.o: lex.uri.c uri.h $(INCDIR)/stack.h
	$(CC) $(CFLAGS) lex.uri.c

lex.uri.c: uri.l
	$(LEX) -Puri uri.l

clean:
	rm -f lex.uri.o lex.uri.c
