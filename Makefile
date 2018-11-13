CC=gcc
CFLAGS:=-g -Wall -Wextra -pedantic $(shell pkg-config --cflags --libs libnl-3.0 libnl-genl-3.0)

all:dave

dave:dave.c

clean:
	$(RM) -f dave
