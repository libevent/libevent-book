#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <event2/buffer.h>
#include <event2/event.h>
#include <event2/http.h>

#define BOOTSTRAP_CDN "https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist"
#define BOOTSTRAP_JS BOOTSTRAP_CDN "/js"
#define BOOTSTRAP_CSS BOOTSTRAP_CDN "/css"

static const struct table_entry {
	const char *extension;
	const char *content_type;
} content_type_table[] = {
	{"txt", "text/plain"},
	{"c", "text/plain"},
	{"h", "text/plain"},
	{"html", "text/html"},
	{"htm", "text/htm"},
	{"css", "text/css"},
	{"gif", "image/gif"},
	{"jpg", "image/jpeg"},
	{"jpeg", "image/jpeg"},
	{"png", "image/png"},
	{"pdf", "application/pdf"},
	{"ps", "application/postscript"},
	{NULL, NULL},
};

static void
add_content_length(struct evhttp_request *req, unsigned len)
{
	char buf[128];

	snprintf(buf, sizeof(buf), "%u", len);
	evhttp_add_header(
		evhttp_request_get_output_headers(req), "Content-Length", buf);
}

#if defined(WIN32)
#define DIR_SEPARATOR '\\'
#else
#define DIR_SEPARATOR '/'
#endif

static void
path_join(char *destination, const char *path1, const char *path2)
{
	if (path1 && *path1) {
		ssize_t len = strlen(path1);
		strcpy(destination, path1);

		if (destination[len - 1] == DIR_SEPARATOR) {
			if (path2 && *path2) {
				strcpy(destination + len,
					(*path2 == DIR_SEPARATOR) ? (path2 + 1) : path2);
			}
		} else {
			if (path2 && *path2) {
				if (*path2 == DIR_SEPARATOR)
					strcpy(destination + len, path2);
				else {
					destination[len] = DIR_SEPARATOR;
					strcpy(destination + len + 1, path2);
				}
			}
		}
	} else if (path2 && *path2)
		strcpy(destination, path2);
	else
		destination[0] = '\0';
}

/* Try to guess the content type of "path" */
static const char *
guess_content_type(const char *path)
{
	const char *last_period, *extension;
	const struct table_entry *ent;
	last_period = strrchr(path, '.');
	if (!last_period || strchr(last_period, '/'))
		goto not_found; /* no exension */
	extension = last_period + 1;
	for (ent = &content_type_table[0]; ent->extension; ++ent) {
		if (!evutil_ascii_strcasecmp(ent->extension, extension))
			return ent->content_type;
	}

not_found:
	return "application/stream";
}

static void
send_file_to_user(struct evhttp_request *req, void *arg)
{
	struct evbuffer *evb = NULL;
	struct evhttp_uri *decoded = NULL;
	struct stat st;
	int fd = -1;
	const char *static_dir = ".";

	enum evhttp_cmd_type cmd = evhttp_request_get_command(req);
	if (cmd != EVHTTP_REQ_GET && cmd != EVHTTP_REQ_HEAD) {
		return;
	}

	/* Decode the URI */
	decoded = evhttp_uri_parse(evhttp_request_get_uri(req));
	if (!decoded) {
		evhttp_send_error(req, HTTP_BADREQUEST, 0);
		return;
	}

	/* Let's see what path the user asked for. */
	const char *path = evhttp_uri_get_path(decoded);
	if (!path)
		path = "/";

	/* We need to decode it, to see what path the user really wanted. */
	char *decoded_path = evhttp_uridecode(path, 0, NULL);
	if (decoded_path == NULL)
		goto err;

	/* Don't allow any ".."s in the path, to avoid exposing stuff outside
	 * of the docroot.  This test is both overzealous and underzealous:
	 * it forbids aceptable paths like "/this/one..here", but it doesn't
	 * do anything to prevent symlink following." */
	if (strstr(decoded_path, ".."))
		goto err;

	char whole_path[PATH_MAX] = {0};
	const char *type = NULL;
	path_join(whole_path, static_dir, decoded_path);
	char *real_file = realpath(whole_path, NULL);
	if (real_file) {
		strncpy(whole_path, real_file, sizeof(whole_path));
		free(real_file);
	} else {
		/* check also if is there gz-ready static version */
		type = guess_content_type(whole_path);

		char gz_path[PATH_MAX + 3] = {0};
		snprintf(gz_path, sizeof(gz_path), "%s.gz", whole_path);
		char *real_file = realpath(gz_path, NULL);
		if (real_file) {
			evhttp_add_header(evhttp_request_get_output_headers(req),
				"Content-Encoding", "gzip");
			strncpy(whole_path, real_file, sizeof(whole_path));
			free(real_file);
		} else {
			fprintf(stderr, "File '%s' not found\n", whole_path);
			evhttp_send_error(req, HTTP_NOTFOUND, NULL);
			goto done;
		}
	}

	if (stat(whole_path, &st) < 0) {
		goto err;
	}

	if ((evb = evbuffer_new()) == NULL) {
		evhttp_send_error(req, HTTP_INTERNAL, 0);
		goto cleanup;
	}

	bool dir_mode = false;

	if (S_ISDIR(st.st_mode)) {
		/* Check if there is index.html file */
		char index_file[PATH_MAX + 11];
		snprintf(index_file, sizeof(index_file), "%s/index.html", whole_path);

		if (stat(index_file, &st) < 0)
			dir_mode = true;
		else
			strcpy(whole_path, index_file);
	}

	if (dir_mode) {
		DIR *d;
		struct dirent *ent;

		const char *trailing_slash = "";

		if (!strlen(path) || path[strlen(path) - 1] != '/')
			trailing_slash = "/";
		if (!(d = opendir(whole_path))) {
			goto err;
		}

		evbuffer_add_printf(evb,
			"<!DOCTYPE html>\n"
			"<html lang=\"en\">"
			"<head>\n"
			"<meta name=\"viewport\" "
			"content=\"width=device-width,initial-scale=1\">\n"
			"<title>%s</title>\n"
			"<link rel=\"shortcut icon\" href=\"/favicon.png\">\n"
			"<link rel=\"stylesheet\" href=\"" BOOTSTRAP_CSS
			"/bootstrap.min.css\">\n"
			"<script src=\"" BOOTSTRAP_JS
			"/bootstrap.bundle.min.js\"></script>\n"
			"<base href='%s%s'>\n"
			"</head>\n"
			"<body id=\"top\">\n"
			"<nav class=\"navbar navbar-expand-lg navbar-dark sticky-top\">\n"
			"<div class=\"container\">\n"
			"</div>\n"
			"</nav>\n"
			"<main>\n"
			"<div class=\"container p-3\">\n"
			"<h2>%s</h2>\n"
			"<ul class=\"list-unstyled my-3\">\n",
			decoded_path, /* XXX html-escape this. */
			path,		  /* XXX html-escape this? */
			trailing_slash, decoded_path /* XXX html-escape this */);
		while ((ent = readdir(d))) {
			const char *name = ent->d_name;
			// ignore '.' directory entries
			if (strcmp(name, ".") == 0)
				continue;
			// show '..' only for subdirs
			if (strcmp(path, "/") == 0 && strcmp(name, "..") == 0)
				continue;
			evbuffer_add_printf(evb, "<li><a href=\"%s\">%s</a>\n", name,
				name); /* XXX escape this */
		}
		evbuffer_add_printf(evb, "</ul>"
								 "</div>\n"
								 "</main>\n"
								 "<footer class=\"p-3\">\n"
								 "<div class=\"container\">\n"
								 "</div>\n"
								 "</footer>\n"
								 "</body>"
								 "</html>\n");
		closedir(d);

		add_content_length(req, evbuffer_get_length(evb));
		if (cmd == EVHTTP_REQ_HEAD)
			evbuffer_drain(evb, evbuffer_get_length(evb));
		evhttp_add_header(evhttp_request_get_output_headers(req),
			"Content-Type", "text/html");
	} else {
		/* Otherwise it's a file; add it to the buffer to get
		 * sent via sendfile */
		if (type == NULL)
			type = guess_content_type(whole_path);
		evhttp_add_header(
			evhttp_request_get_output_headers(req), "Content-Type", type);

		if (st.st_size != 0) {
			if ((fd = open(whole_path, O_RDONLY)) == -1) {
				if (errno == ENOENT) {
					fprintf(stderr, "File '%s' not found\n", whole_path);
					evhttp_send_error(req, HTTP_NOTFOUND, NULL);
				} else {
					evhttp_send_error(req, HTTP_INTERNAL, NULL);
				}
				goto done;
			}
			if (cmd != EVHTTP_REQ_HEAD) {
				if (evbuffer_add_file(evb, fd, 0, st.st_size) != 0) {
					evhttp_send_error(req, HTTP_INTERNAL, NULL);
					goto cleanup;
				}
			}
		}
		add_content_length(req, st.st_size);
	}
	evhttp_send_reply(req, HTTP_OK, "OK", evb);

	goto done;

err:
	evhttp_send_error(req, HTTP_NOTFOUND, NULL);
cleanup:
	if (fd >= 0)
		close(fd);

done:
	if (decoded)
		evhttp_uri_free(decoded);
	if (decoded_path)
		free(decoded_path);
	if (evb)
		evbuffer_free(evb);
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
	evhttp_set_gencb(http_server, send_file_to_user, NULL);

	sig_int = evsignal_new(base, SIGINT, signal_cb, base);
	event_add(sig_int, NULL);

	printf("Listening requests on http://%s:%d\n", http_addr, http_port);

	event_base_dispatch(base);

	evhttp_free(http_server);
	event_free(sig_int);
	event_base_free(base);
}
