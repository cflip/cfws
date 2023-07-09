#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "file.h"
#include "http.h"
#include "net.h"

#define CFWS_DEFAULT_PORT 8080

static int serverfd;

static void handle_request(const struct http_request *, int);

static void handle_sigint(int signum)
{
	(void)signum;
	shutdown(serverfd, SHUT_RDWR);
}

int main(int argc, char *argv[])
{
	int port = CFWS_DEFAULT_PORT;
	int clientfd;
	struct http_request request;

	signal(SIGINT, handle_sigint);

	/* Prevent the program from quitting if it attempts to write to a closed
	 * socket. */
	signal(SIGPIPE, SIG_IGN);

	serverfd = net_init_server(port);
	if (serverfd == -1)
		return 1;

	printf("Serving a directory at localhost:%d\n", port);

	while (1) {
		int rc = net_next_request(serverfd, &clientfd, &request);
		if (rc != 0)
			break;

		handle_request(&request, clientfd);

		http_free_request(&request);
		close(clientfd);
	}

	close(serverfd);
	return 0;
}

static void handle_request(const struct http_request *req, int sockfd)
{
	char *filepath = NULL;
	enum http_res_code res_code;
	enum serve_method method;

	if (req->method == HTTP_METHOD_UNKNOWN) {
		method = SERVE_METHOD_ERROR;
		res_code = HTTP_RESPONSE_NOTIMPLEMENTED;
	} else if (req->uri == NULL) {
		method = SERVE_METHOD_ERROR;
		res_code = HTTP_RESPONSE_BADREQUEST;
	} else {
		/* Find the local path for the resource and decide how to serve it. */
		filepath = file_path_for_uri(req->uri);
		method = file_method_for_path(filepath, &res_code);
	}

	/* Write the status line and (TODO) extra headers */
	http_response_statusline(res_code, sockfd);

	/* Use the chosen method to fill in the rest of the response */
	switch (method) {
	case SERVE_METHOD_FILE:
		file_read(filepath, sockfd);
		break;
	case SERVE_METHOD_PHP:
		file_read_php(filepath, req, sockfd);
		break;
	case SERVE_METHOD_ERROR: {
		const char *errmsg = "Content-Type: text/plain\r\n\r\nEpic fail";
		write(sockfd, errmsg, strlen(errmsg));
		break;
	}
	}

	if (filepath)
		free(filepath);
}
