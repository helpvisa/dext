.POSIX:
.SUFFIXES:
CC = cc
LDLIBS = -lncurses
LDFLAGS =
CFLAGS =
PREFIX = /usr/local

all: main.o parse.o
	$(CC) -Wall $(LDFLAGS) -o dext main.o create_structures.o helpers.o buffers.o $(LDLIBS)
	$(CC) -Wall $(LDFLAGS) -o dewc parse.o create_structures.o helpers.o $(LDLIBS)

main.o: main.c create_structures.o buffers.o
parse.o: parse.c create_structures.o 
create_structures.o: create_structures.c structs.h helpers.o
buffers.o: buffers.c
helpers.o: helpers.c

install: all
	cp dext $(PREFIX)/bin/
	cp dewc $(PREFIX)/bin/

clean:
	rm -f ./dext
	rm -f ./dewc
	rm -f ./*.o

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $<
