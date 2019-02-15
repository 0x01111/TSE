SUBDIR = index ChSeg
CXX = g++
LD = g++
#INCDIR = $(TOPDIR)/include

OBJS1 = Md5.o Url.o Document.o StrFun.o Query.o DisplayRst.o
OBJS2 = ./ChSeg/HzSeg.o ./ChSeg/Dict.o

EXECS = DocIndex DocSegment TSESearch Snapshot \
	CrtForwardIdx CrtInvertedIdx

CXXFLAGS = -c
ifeq ($(DEBUG),y)
        CXXFLAGS += -g -Wall
else
        CXXFLAGS += -O2
endif

CXXFLAGS += -I$(INCDIR)
