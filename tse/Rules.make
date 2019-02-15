SUBDIR = uri hlink stack lib
LEX = flex
CC = gcc
CXX = g++
LD = g++
LDLIB = -lpthread -lz
INCDIR = $(TOPDIR)/include

OBJS = $(TOPDIR)/uri/lex.uri.o $(TOPDIR)/hlink/lex.hlink.o \
	$(TOPDIR)/stack/stack.o $(TOPDIR)/lib/misc.o \
	Crawl.o Search.o DataEngine.o DatabaseEngine.o \
	FileEngine.o TianwangFile.o IsamFile.o Link4SEFile.o \
	StrFun.o Url.o Page.o Http.o Md5.o Res.o

EXECS = Tse Stat tfind tfindForeign

CFLAGS = -c
ifeq ($(DEBUG),y)
	CFLAGS += -g -Wall
else
	CFLAGS += -O2
endif
CFLAGS += -I$(INCDIR)
