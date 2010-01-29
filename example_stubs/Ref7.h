/* Includes and stub declarations to make some examples in Ref2 work */

#include <stdlib.h>
#include <event2/buffer.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
struct evbuffer *buf = NULL;

int consume(const char *, size_t len);
int generate_data(char *, size_t len);
