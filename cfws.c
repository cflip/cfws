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

int flag_verbose = 0;

static void handle_request(const struct http_request *, int);

static void handle_sigint(int signum)
{
	(void)signum;
	shutdown(serverfd, SHUT_RDWR);
}

static void print_usage(const char *program_name)
{
	printf("USAGE: %s [OPTION]\n", program_name);
	printf("\t-h\tPrint this help and exit\n\t-v\tPrint detailed information for each request and response\n");
}

int main(int argc, char *argv[])
{
	int port = CFWS_DEFAULT_PORT;
	int clientfd;
	struct http_request request;

	int optchar;
	while ((optchar = getopt(argc, argv, "hv")) != -1) {
		switch (optchar) {
		case 'h':
			print_usage(argv[0]);
			exit(EXIT_SUCCESS);
		case 'v':
			flag_verbose = 1;
			break;
		default:
			print_usage(argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	signal(SIGINT, handle_sigint);

	/* Prevent the program from quitting if it attempts to write to a closed
	 * socket. */
	signal(SIGPIPE, SIG_IGN);

	serverfd = net_init_server(port);
	if (serverfd == -1)
		return 1;

	if (flag_verbose)
		printf("Serving a directory at localhost:%d\n", port);

	while (1) {
		int rc = net_next_request(serverfd, &clientfd, &request);
		if (rc != 0)
			break;

		handle_request(&request, clientfd);

		http_free_request(&request);
		close(clientfd);
	}

	if (flag_verbose)
		printf("Shutting down server...\n");

	close(serverfd);
	return 0;
}

static void handle_request(const struct http_request *req, int sockfd)
{
	char *filepath = NULL;
	enum http_res_code res_code;
	enum serve_method method;

	if (flag_verbose) {
		switch (req->method) {
		case HTTP_METHOD_GET:
			printf("GET  :: %s\n", req->uri);
			break;
		case HTTP_METHOD_POST:
			printf("POST :: %s\n", req->uri);
			break;
		default:
			printf("???  :: %s\n", req->uri);
			break;
		}
	}

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
		if (flag_verbose)
			printf(" -> FILE %s\n", filepath);
		file_read(filepath, sockfd);
		break;
	case SERVE_METHOD_PHP:
		if (flag_verbose)
			printf(" -> PHP  %s\n", filepath);
		file_read_php(filepath, req, sockfd);
		break;
	case SERVE_METHOD_ERROR: {
		if (flag_verbose)
			printf(" -> ERROR %d\n", res_code);
		const char *errmsg = "Content-Type: text/plain\r\n\r\nEpic fail";
		write(sockfd, errmsg, strlen(errmsg));
		break;
	}
	}

	if (filepath)
		free(filepath);
}
