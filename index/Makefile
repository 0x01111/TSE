TOPDIR = $(shell pwd)
include $(TOPDIR)/Rules.make

.PHONY: all clean

all: $(EXECS) ChSeg

DocIndex: DocIndex.o $(OBJS1)
	$(LD) -o DocIndex $^
DocSegment: DocSegment.o $(OBJS1) $(OBJS2)
	$(LD) -o DocSegment $^ 
TSESearch: TSESearch.o $(OBJS1) $(OBJS2)
	$(LD) -o TSESearch $^
Snapshot: Snapshot.o $(OBJS1)
	$(LD) -o Snapshot $^

CrtForwardIdx: CrtForwardIdx.o
	$(LD) -o CrtForwardIdx $^
CrtInvertedIdx: CrtInvertedIdx.o
	$(LD) -o CrtInvertedIdx $^

ChSeg:
	cd ChSeg; make; cd ..

clean:
	rm -f *.o $(EXECS) core.*
	cd ChSeg; make clean


#;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#       make relase version
#;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
distribute:
	@(mkdir index) ||exit 1; \
	(cp -r Rules.make Makefile *.cpp *.c *.h \
	./Data ./ChSeg words.dict\
	readme.txt ./public_html ./index) ||exit 1; \
	tarball="index.`date '+%y%m%d-%H%M'`.`uname`.tar"; \
	echo @create release tar ball as $$tarball.gz......     ; \
	rm -f $$tarball; \
	tar cf $$tarball ./index; \
	gzip $$tarball; \
	rm -rf ./index; \
	echo @done!
