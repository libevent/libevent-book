#include <string.h>
#include <signal.h>
#include <event2/buffer.h>
#include <event2/event.h>
#include <event2/http.h>

static void
generic_request_handler(struct evhttp_request *req, void *ctx)
{
	struct evbuffer *reply = evbuffer_new();

	evbuffer_add_printf(reply, "It works!");
	evhttp_send_reply(req, HTTP_OK, NULL, reply);
	evbuffer_free(reply);
}

static void
signal_cb(evutil_socket_t fd, short event, void *arg)
{
	printf("%s signal received\n", strsignal(fd));
	event_base_loopbreak(arg);
}

int
main()
{
	ev_uint16_t http_port = 8080;
	char *http_addr = "0.0.0.0";
	struct event_base *base;
	struct evhttp *http_server;
	struct event *sig_int;

	base = event_base_new();

	http_server = evhttp_new(base);
	evhttp_bind_socket(http_server, http_addr, http_port);
	evhttp_set_gencb(http_server, generic_request_handler, NULL);

	sig_int = evsignal_new(base, SIGINT, signal_cb, base);
	event_add(sig_int, NULL);

	printf("Listening requests on http://%s:%d\n", http_addr, http_port);

	event_base_dispatch(base);

	evhttp_free(http_server);
	event_free(sig_int);
	event_base_free(base);
}
