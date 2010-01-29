
CC=gcc
CFLAGS=-g -Wall $(LEBOOK_CFLAGS)

EXAMPLE_BINARIES=R8_echo_server

all: examples

examples: $(EXAMPLE_BINARIES)

R8_echo_server: R8_echo_server.o
	$(CC) $(CFLAGS) R8_echo_server.o -o R8_echo_server -levent_core

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *~
	rm -f *.o
	rm -f $(EXAMPLE_BINARIES)
