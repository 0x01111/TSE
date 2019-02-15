SUBDIR = index ChSeg
CXX = g++
LD = g++
#INCDIR = $(TOPDIR)/include

OBJS = HzSeg.o Dict.o

EXECS = ExSeg ExSegUrl

CXXFLAGS = -c
ifeq ($(DEBUG),y)
        CXXFLAGS += -g -Wall
else
        CXXFLAGS += -O2
endif

CXXFLAGS += -I$(INCDIR)
