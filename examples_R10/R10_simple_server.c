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

int
main()
{
	ev_uint16_t http_port = 8080;
	char *http_addr = "0.0.0.0";
	struct event_base *base;
	struct evhttp *http_server;

	base = event_base_new();

	http_server = evhttp_new(base);
	evhttp_bind_socket(http_server, http_addr, http_port);
	evhttp_set_gencb(http_server, generic_request_handler, NULL);

	printf("Listening requests on http://%s:%d\n", http_addr, http_port);

	event_base_dispatch(base);

	evhttp_free(http_server);
	event_base_free(base);
}
