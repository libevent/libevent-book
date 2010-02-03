
CC=gcc
CFLAGS=-g -Wall $(LEBOOK_CFLAGS)

EXAMPLE_BINARIES=R6_http_client

all: examples

examples: $(EXAMPLE_BINARIES)

R6_http_client: R6_http_client.o
	$(CC) $(CFLAGS) R6_http_client.o -o R6_http_client -levent

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *~
	rm -f *.o
	rm -f $(EXAMPLE_BINARIES)
