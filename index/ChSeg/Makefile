TOPDIR = $(shell pwd)
include $(TOPDIR)/Rules.make

.PHONY: all clean

all: $(EXECS)

ExSeg: ExSeg.o $(OBJS)
	$(LD) -o ExSeg $^
ExSegUrl: ExSegUrl.o $(OBJS)
	$(LD) -o ExSegUrl $^

clean:
	rm -f *.o $(EXECS) core.* *.seg
