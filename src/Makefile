
CC=clang
CFLAGS=-I. --std=c99

all: test libempack.a

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

libempack.a: empack.o
	ar rcs $@ $< 

test: test.c empack.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

.PHONY: test libempack.a

