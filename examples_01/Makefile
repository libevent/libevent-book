
CC=gcc
CFLAGS=-g -Wall -L/Users/nickm/le/lib -I/Users/nickm/le/include

GENERATED_HTML=01_intro.html
EXAMPLE_BINARIES=01_sync_webclient 01_rot13_server_forking \
	01_rot13_server_select 01_rot13_server_libevent \
	01_rot13_server_bufferevent

all: examples

examples: $(EXAMPLE_BINARIES)

01_intro.html: 01_sync_webclient.c

01_sync_webclient: 01_sync_webclient.o
	$(CC) $(CFLAGS) 01_sync_webclient.o -o 01_sync_webclient

01_rot13_server_forking: 01_rot13_server_forking.o
	$(CC) $(CFLAGS) 01_rot13_server_forking.o -o 01_rot13_server_forking

01_rot13_server_select: 01_rot13_server_select.o
	$(CC) $(CFLAGS) 01_rot13_server_select.o -o 01_rot13_server_select

01_rot13_server_libevent: 01_rot13_server_libevent.o
	$(CC) $(CFLAGS) 01_rot13_server_libevent.o -o 01_rot13_server_libevent -levent_core

01_rot13_server_bufferevent: 01_rot13_server_bufferevent.o
	$(CC) $(CFLAGS) /Users/nickm/le/lib/libevent_core.a 01_rot13_server_bufferevent.o -o 01_rot13_server_bufferevent

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *~
	rm -f *.o
	rm -f $(EXAMPLE_BINARIES)
	rm -f $(GENERATED_HTML)
