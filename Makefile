.POSIX:
.SUFFIXES:
CC = cc
LDLIBS = -lncurses
LDFLAGS =
CFLAGS =
OUTPUT = dext
PREFIX = /usr/local

all: main.o
	$(CC) -Wall $(LDFLAGS) -o $(OUTPUT) main.o $(LDLIBS)
main.o: main.c

install: all
	cp $(OUTPUT) $(PREFIX)/bin/

clean:
	rm -f ./$(OUTPUT)
	rm -f ./*.o

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $<
