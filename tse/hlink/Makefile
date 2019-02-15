TOPDIR = $(shell cd ..; pwd)
include $(TOPDIR)/Rules.make

lex.hlink.o: lex.hlink.c hlink.h $(INCDIR)/misc.h $(INCDIR)/uri.h
	$(CC) $(CFLAGS) lex.hlink.c

lex.hlink.c: hlink.l
	$(LEX) -Phlink hlink.l

clean:
	rm -f lex.hlink.o lex.hlink.cc

