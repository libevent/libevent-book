
CC=gcc
CFLAGS=-g -Wall $(LEBOOK_CFLAGS)

EXAMPLE_BINARIES=R10_simple_server R10_static_server

all: examples

examples: $(EXAMPLE_BINARIES)

R10_simple_server: R10_simple_server.o
	$(CC) $(CFLAGS) R10_simple_server.o -o R10_simple_server -levent

R10_static_server: R10_static_server.o
	$(CC) $(CFLAGS) R10_static_server.o -o R10_static_server -levent

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *~
	rm -f *.o
	rm -f $(EXAMPLE_BINARIES)
