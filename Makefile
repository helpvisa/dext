.POSIX:
.SUFFIXES:
CC = cc
LDLIBS = -lcurses
LDFLAGS =
CFLAGS = -ansi -pedantic
PREFIX = /usr/local

all: main.o
	$(CC) -Wall $(LDFLAGS) -o dext main.o buffers.o helpers.o render.o $(LDLIBS)

main.o: main.c buffers.o helpers.o render.o
buffers.o: buffers.c buffers.h
helpers.o: helpers.c helpers.h
render.o: render.c render.h

install: all
	cp dext $(PREFIX)/bin/
	cp dewc $(PREFIX)/bin/

clean:
	rm -f ./dext
	rm -f ./*.o

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $<
