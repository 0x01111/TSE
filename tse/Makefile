TOPDIR = $(shell pwd)
include $(TOPDIR)/Rules.make

.PHONY: all clean

all: subdir $(EXECS)

subdir:
	for n in $(SUBDIR); do $(MAKE) -C $$n || exit 1; done
Tse: Main.o $(OBJS)
	$(LD) -o Tse Main.o $(OBJS) $(LDLIB) 
Stat:	Stat.cpp
	$(LD) -o Stat  Stat.cpp
tfind:	tfind.cpp
	$(LD) -o tfind tfind.cpp
tfindForeign: tfindForeign.cpp
	$(LD) -o tfindForeign tfindForeign.cpp

clean:
	rm -f *.o $(EXECS) core.* WebData.db WebData.idx \
		Tianwang.raw.* \
		logfile *.gz
	for n in $(SUBDIR); do $(MAKE) -C $$n clean; done


Main.o:  Main.cpp
	$(CXX) $(CFLAGS) -c $^
Crawl.o: Crawl.cpp
	$(CXX) $(CFLAGS) -c $^
Search.o: Search.cpp
	$(CXX) $(CFLAGS) -c $^
DataEngine.o: DataEngine.cpp
	$(CXX) $(CFLAGS) -c $^
DatabaseEngine.o: DatabaseEngine.cpp
	$(CXX) $(CFLAGS) -c $^
FileEngine.o: FileEngine.cpp
	$(CXX) $(CFLAGS) -c $^
TianwangFile.o: TianwangFile.cpp
	$(CXX) $(CFLAGS) -c $^
IsamFile.o: IsamFile.cpp
	$(CXX) $(CFLAGS) -c $^
Link4SEFile.o: Link4SEFile.cpp
	$(CXX) $(CFLAGS) -c $^
StrFun.o: StrFun.cpp
	$(CXX) $(CFLAGS) -c $^
Url.o: Url.cpp
	$(CXX) $(CFLAGS) -c $^
Page.o: Page.cpp
	$(CXX) $(CFLAGS) -c $^
Http.o: Http.cpp
	$(CXX) $(CFLAGS) -c $^
Md5.o: Md5.cpp
	$(CXX) $(CFLAGS) -c $^
Res.o: Res.cpp
	$(CXX) $(CFLAGS) -c $^


#;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#       make relase version
#;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
distribute:
	@(mkdir tse) ||exit 1; \
	(cp -r Rules.make Makefile *.cpp *.h *.sh *seed* tse_ipblock pku.hosts\
	tse_unreachHost.list *.txt README \
	hlink include lib stack uri ./tse) ||exit 1; \
	tarball="tse.`date '+%y%m%d-%H%M'`.`uname`.tar"; \
	echo @create release tar ball as $$tarball.gz......     ; \
	rm -f $$tarball; \
	tar cf $$tarball ./tse; \
	gzip $$tarball; \
	rm -rf ./tse; \
	echo @done!
