.POSIX:
.SUFFIXES:
CC = cc
LDLIBS = -lncurses
LDFLAGS =
CFLAGS =
PREFIX = /usr/local

all: main.o buffers.o
	$(CC) -Wall $(LDFLAGS) -o dext main.o buffers.o $(LDLIBS)

main.o: main.c buffers.o
buffers.o: buffers.c

install: all
	cp dext $(PREFIX)/bin/
	cp dewc $(PREFIX)/bin/

clean:
	rm -f ./dext
	rm -f ./*.o

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $<
