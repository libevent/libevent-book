
CC=gcc
CFLAGS=-g -Wall $(LEBOOK_CFLAGS)

EXAMPLE_BINARIES=01_sync_webclient 01_rot13_server_forking \
	01_rot13_server_select 01_rot13_server_libevent \
	01_rot13_server_bufferevent

all: examples

examples: $(EXAMPLE_BINARIES)

01_sync_webclient: 01_sync_webclient.o
	$(CC) $(CFLAGS) 01_sync_webclient.o -o 01_sync_webclient

01_rot13_server_forking: 01_rot13_server_forking.o
	$(CC) $(CFLAGS) 01_rot13_server_forking.o -o 01_rot13_server_forking

01_rot13_server_select: 01_rot13_server_select.o
	$(CC) $(CFLAGS) 01_rot13_server_select.o -o 01_rot13_server_select

01_rot13_server_libevent: 01_rot13_server_libevent.o
	$(CC) $(CFLAGS) 01_rot13_server_libevent.o -o 01_rot13_server_libevent -levent_core

01_rot13_server_bufferevent: 01_rot13_server_bufferevent.o
	$(CC) $(CFLAGS)  01_rot13_server_bufferevent.o -o 01_rot13_server_bufferevent -levent_core

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *~
	rm -f *.o
	rm -f $(EXAMPLE_BINARIES)
