# davep 20191123 ; tinkering with libevent and libev
# http://www.wangafu.net/~nickm/libevent-book/TOC.html
# http://software.schmorp.de/pkg/libev.html
# https://github.com/P-p-H-d/mlib
#
PKGS=libnl-3.0 libnl-genl-3.0 libevent_core libevent libevent_extra
CFLAGS:=-g -Wall -Wpedantic -Wextra $(shell pkg-config --cflags $(PKGS))
LDFLAGS:=-g $(shell pkg-config --libs $(PKGS))

all:scan-event-ev

scan-event: scan-event.o hdump.o iw-scan.o nlnames.o
	$(CC) $(LDFLAGS) -o $@ $^

scan-event.o: scan-event.c
	$(CC) $(CFLAGS) -c scan-event.c -o scan-event.o 

scan-event-ev: scan-event-ev.o hdump.o iw-scan.o nlnames.o xassert.o bytebuf.o
	$(CC) $(LDFLAGS) -lev -o $@ $^

scan-event-ev.o: scan-event-ev.c iw-scan.h xassert.h bytebuf.h log.h
	$(CC) $(CFLAGS) -c $< -o $@

hdump.o:hdump.c hdump.h

bytebuf.o:bytebuf.c bytebuf.h xassert.h log.h
xassert.o:xassert.c xassert.h
iw-scan.o:iw-scan.c iw-scan.h bytebuf.h xassert.h log.h

nlnames.o:nlnames.c

test_xassert:test_xassert.o xassert.o
test_xassert.o:test_xassert.c xassert.h

test_bytebuf:test_bytebuf.o bytebuf.o xassert.o hdump.o
test_bytebuf.o:test_bytebuf.c bytebuf.h xassert.h log.h

test: test_bytebuf
	valgrind --leak-check=yes ./test_bytebuf

clean:
	$(RM) *.o scan-event scan-event-ev
