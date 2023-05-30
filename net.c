#include "net.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "file.h"
#include "http.h"

int initialize_server(int port)
{
	struct sockaddr_in addr;
	int rc;
	int sockopts = 1;
	int sockfd;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("Failed to create server socket");
		return -1;
	}

	/* Allow the port to be reused, prevents errors when quickly starting
	 * and restarting the server. */
	/* TODO: Also use SO_REUSEPORT? */
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopts, sizeof(int));

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port        = htons(port);

	rc = bind(sockfd, (struct sockaddr*)&addr, sizeof(struct sockaddr));
	if (rc == -1) {
		perror("Failed to bind server socket");
		return -1;
	}

	rc = listen(sockfd, CFWS_MAXCONN);
	if (rc == -1) {
		perror("Failed to listen on server socket");
		return -1;
	}

	return sockfd;
}

void handle_connection(int connfd)
{
	char readbuf[CFWS_MAXREAD];
	struct http_request req;

	char *filepath;
	enum http_res_code res_code;
	enum serve_method method;

	/* Read and parse the HTTP request */
	memset(readbuf, 0, CFWS_MAXREAD);
	read(connfd, readbuf, CFWS_MAXREAD - 1);
	req = http_parse_request(readbuf);

	/* Get the local file path for the given URI. */
	filepath = file_path_for_uri(req.uri);

	printf("GET %s : %s\n", req.uri, filepath);

	/* Determine the method that should be used to serve this file */
	method = file_method_for_path(filepath, &res_code);

	/* Write the HTTP response status and any required headers */
	http_response_statusline(res_code, connfd);

	/* Use the chosen method to fill in the rest of the response */
	switch (method) {
	case SERVE_METHOD_FILE:
		file_read(filepath, connfd);
		break;
	case SERVE_METHOD_PHP:
		file_read_php(filepath, connfd);
		break;
	case SERVE_METHOD_ERROR: {
		const char *errmsg = "Content-Type: text/plain\r\n\r\nEpic fail";
		write(connfd, errmsg, strlen(errmsg));
		break;
	}
	}

	free(filepath);
	http_free_request(&req);
}
