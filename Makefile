.POSIX:
.SUFFIXES:
CC = cc
LDLIBS = -lncurses
LDFLAGS =
CFLAGS =
PREFIX = /usr/local

all: main.o parse.o
	$(CC) -Wall $(LDFLAGS) -o dext main.o $(LDLIBS)
	$(CC) -Wall $(LDFLAGS) -o structure-parser parse.o create_structures.o $(LDLIBS)

main.o: main.c
parse.o: parse.c create_structures.o 
create_structures.o: create_structures.c structs.h

install: all
	cp dext $(PREFIX)/bin/

clean:
	rm -f ./dext
	rm -f ./structure-parser
	rm -f ./*.o

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $<
