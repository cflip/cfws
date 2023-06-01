#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "file.h"
#include "http.h"
#include "net.h"

#define CFWS_DEFAULT_PORT 8080

static void handle_request(const struct http_request *, int);

int main(int argc, char *argv[])
{
	int port = CFWS_DEFAULT_PORT;
	int serverfd, clientfd;
	struct http_request request;

	serverfd = net_init_server(port);
	if (serverfd == -1)
		return 1;

	printf("Serving a directory at localhost:%d\n", port);

	while (1) {
		request = net_next_request(serverfd, &clientfd);

		handle_request(&request, clientfd);

		http_free_request(&request);
		close(clientfd);
	}

	close(serverfd);
	return 0;
}

static void handle_request(const struct http_request *req, int sockfd)
{
	char *filepath;
	enum http_res_code res_code;
	enum serve_method method;

	/* Find the local path for the resource and decide how to serve it. */
	filepath = file_path_for_uri(req->uri);
	method = file_method_for_path(filepath, &res_code);

	/* Write the status line and (TODO) extra headers */
	http_response_statusline(res_code, sockfd);

	/* Use the chosen method to fill in the rest of the response */
	switch (method) {
	case SERVE_METHOD_FILE:
		file_read(filepath, sockfd);
		break;
	case SERVE_METHOD_PHP:
		file_read_php(filepath, req->query_str, sockfd);
		break;
	case SERVE_METHOD_ERROR: {
		const char *errmsg = "Content-Type: text/plain\r\n\r\nEpic fail";
		write(sockfd, errmsg, strlen(errmsg));
		break;
	}
	}

	free(filepath);
}
