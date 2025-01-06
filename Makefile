.POSIX:
.SUFFIXES:

CC = cc
LDLIBS = -lcurses
LDFLAGS =
CFLAGS =
PREFIX = /usr/local

VPATH = src
BUILDDIR = build

all: build $(BUILDDIR)/main.o
	$(CC) -Wall $(LDFLAGS) -o $(BUILDDIR)/dext \
		$(BUILDDIR)/main.o \
		$(BUILDDIR)/buffers.o \
		$(BUILDDIR)/helpers.o \
		$(BUILDDIR)/render.o \
		$(LDLIBS)

build:
	mkdir -p build

$(BUILDDIR)/%.o: %.c
	$(CC) -c $< -o $@

$(BUILDDIR)/main.o: main.c \
	$(BUILDDIR)/buffers.o \
	$(BUILDDIR)/helpers.o \
	$(BUILDDIR)/render.o
$(BUILDDIR)/buffers.o: buffers.c buffers.h
$(BUILDDIR)/helpers.o: helpers.c helpers.h
$(BUILDDIR)/render.o: render.c render.h

install: all
	cp dext $(PREFIX)/bin/
	cp dewc $(PREFIX)/bin/

clean:
	rm -rf $(BUILDDIR)
