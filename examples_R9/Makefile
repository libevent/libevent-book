
CC=gcc
CFLAGS=-g -Wall $(LEBOOK_CFLAGS)

EXAMPLE_BINARIES=R9_multilookup R9_dns_server

all: examples

examples: $(EXAMPLE_BINARIES)

R9_multilookup: R9_multilookup.o
	$(CC) $(CFLAGS) R9_multilookup.o -o R9_multilookup -levent

R9_dns_server: R9_dns_server.o
	$(CC) $(CFLAGS) R9_dns_server.o -o R9_dns_server -levent

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *~
	rm -f *.o
	rm -f $(EXAMPLE_BINARIES)
