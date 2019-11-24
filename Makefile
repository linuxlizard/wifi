# davep 20191123 ; tinkering with libevent and libev
# http://www.wangafu.net/~nickm/libevent-book/TOC.html
# http://software.schmorp.de/pkg/libev.html
PKGS=libnl-3.0 libnl-genl-3.0 libevent_core libevent libevent_extra
CFLAGS:=-g -Wall -Wpedantic -Wextra $(shell pkg-config --cflags $(PKGS))
LDFLAGS:=-g $(shell pkg-config --libs $(PKGS))

all:scan-event scan-event-ev

scan-event: scan-event.o hdump.o iw-scan.o nlnames.o
	$(CC) $(LDFLAGS) -o $@ $^

scan-event.o: scan-event.c
	$(CC) $(CFLAGS) -c scan-event.c -o scan-event.o 

scan-event-ev: scan-event-ev.o hdump.o iw-scan.o nlnames.o
	$(CC) $(LDFLAGS) -lev -o $@ $^

scan-event-ev.o: scan-event-ev.c
	$(CC) $(CFLAGS) -c $< -o $@

hdump.o:hdump.c

nlnames.o:nlnames.c

clean:
	$(RM) *.o scan-event scan-event-ev
