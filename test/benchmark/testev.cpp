/*
 * echo-server.c
 *
 * This is a modified version of the "simpler ROT13 server with
 * Libevent" from:
 * http://www.wangafu.net/~nickm/libevent-book/01_intro.html
 */

#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define MAX_LINE 16384

void
on_read(struct bufferevent *bev, void *ctx)
{
     struct evbuffer *input, *output;
     char *line;
     size_t n;
     int i;

     input = bufferevent_get_input(bev);
     output = bufferevent_get_output(bev);

     while ((line = evbuffer_readln(input, &n, EVBUFFER_EOL_LF))) {
	  evbuffer_add(output, line, n);
	  evbuffer_add(output, "\n", 1);
	  free(line);
     }

     if (evbuffer_get_length(input) >= MAX_LINE) {
	  /* line is too long */
	  char buf[1024];
	  while (evbuffer_get_length(input)) {
	       int n = evbuffer_remove(input, buf, sizeof(buf));
	       evbuffer_add(output, buf, n);
	  }
	  evbuffer_add(output, "\n", 1);
     }
}

void
on_error(struct bufferevent *bev, short error, void *ctx)
{
     if (error & BEV_EVENT_EOF) {
     } else if (error & BEV_EVENT_ERROR) {
     } else if (error & BEV_EVENT_TIMEOUT) {
     }
     bufferevent_free(bev);
}

void
on_accept(evutil_socket_t listener, short event, void *arg)
{
     struct event_base *base = (event_base*)arg;
     struct sockaddr_storage ss;
     socklen_t slen = sizeof(ss);
     int fd = accept(listener, (struct sockaddr*)&ss, &slen);
     if (fd < 0) {
	  perror("accept");
     } else if (fd > FD_SETSIZE) {
	  close(fd);
     } else {
	  struct bufferevent *bev;
	  evutil_make_socket_nonblocking(fd);
	  bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
	  bufferevent_setcb(bev, on_read, NULL, on_error, NULL);
	  bufferevent_setwatermark(bev, EV_READ, 0, MAX_LINE);
	  bufferevent_enable(bev, EV_READ | EV_WRITE);
     }
}

int
main(int argc, char **argv)
{
     setvbuf(stdout, NULL, _IONBF, 0);

     evutil_socket_t listener;
     struct sockaddr_in sin;
     struct event_base *base;
     struct event *listener_event;

     base = event_base_new();
     if (!base)
	  return 1;

     sin.sin_family = AF_INET;
     sin.sin_addr.s_addr = 0;
     sin.sin_port = htons(8888);

     listener = socket(AF_INET, SOCK_STREAM, 0);
     evutil_make_socket_nonblocking(listener);
     int yes = 1;
     // lose the pesky "address already in use" error message
     setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

     /* win32 junk? */

     if (bind(listener, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
	  perror("bind");
	  return 1;
     }

     if (listen(listener, 16) < 0) {
	  perror("listen");
	  return 1;
     }

     listener_event = event_new(base, listener, EV_READ | EV_PERSIST, on_accept, (void *)base);
     /* check it? */
     event_add(listener_event, NULL);

     event_base_dispatch(base);

     return 0;
}