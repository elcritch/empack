
CC=clang
CFLAGS=-I. --std=c99

all: test libempack.a

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

libempack.a: empack.o em_buffer.o
	ar rcs $@ $< 

test: test.c empack.o em_buffer.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -f *.o *.a

.PHONY: test libempack.a

