.POSIX:
.SUFFIXES:

CC = cc
LDLIBS = -lcurses
LDFLAGS =
CFLAGS =
PREFIX = /usr/local

BUILDDIR = build
SRCDIR = src

SRC = $(wildcard $(SRCDIR)/*.c)
OBJ = $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SRC))

all: build $(OBJ)
	$(CC) -Wall $(LDFLAGS) -o $(BUILDDIR)/dext \
		$(BUILDDIR)/main.o \
		$(BUILDDIR)/buffers.o \
		$(BUILDDIR)/helpers.o \
		$(BUILDDIR)/render.o \
		$(LDLIBS)

build:
	mkdir -p build

run: all
	./build/dext

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

install: all
	cp dext $(PREFIX)/bin/

clean:
	rm -rf $(BUILDDIR)
