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
		$(BUILDDIR)/stringop.o \
		$(BUILDDIR)/io.o \
		$(LDLIBS)

build:
	mkdir -p build

run:
	./$(BUILDDIR)/dext

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

install:
	cp $(BUILDDIR)/dext $(PREFIX)/bin/

clean:
	rm -rf $(BUILDDIR)
