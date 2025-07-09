CC=gcc
CFLAGS=-g -Og -Wall -Werror
LDFLAGS=-g

OBJECTS= \
	build/main.o \
	build/server.o \
	build/controller.o

all: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o build/snesid $(OBJECTS)

build/%.o:src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -fv build/*
