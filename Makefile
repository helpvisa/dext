.POSIX:
.SUFFIXES:
CC = cc
LDLIBS = -lncurses
LDFLAGS =
CFLAGS =
PREFIX = /usr/local

all: main.o
	$(CC) -Wall $(LDFLAGS) -o dext main.o buffers.o helpers.o $(LDLIBS)

main.o: main.c buffers.o helpers.o
buffers.o: buffers.c buffers.h
helpers.o: helpers.c helpers.h

install: all
	cp dext $(PREFIX)/bin/
	cp dewc $(PREFIX)/bin/

clean:
	rm -f ./dext
	rm -f ./*.o

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $<
