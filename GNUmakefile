.POSIX:
.SUFFIXES:
.SUFFIXES: .c .o
.PHONY: all install run clean

SHELL = /bin/sh

CC = cc
LDLIBS = -lcurses
LDFLAGS =
CFLAGS =
PREFIX = /usr/local

BUILDDIR = build
SRCDIR = src

SRC = $(wildcard $(SRCDIR)/*.c)
OBJ = $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SRC))

all: build

run: build
	./$(BUILDDIR)/dext

build: $(OBJ)
	$(CC) -Wall $(LDFLAGS) -o $(BUILDDIR)/dext $(OBJ) $(LDLIBS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

install: build
	install -m 0755 $(BUILDDIR)/dext $(PREFIX)/bin/

clean:
	rm -rf $(BUILDDIR)
