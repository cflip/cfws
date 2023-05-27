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
	char *content_buf;
	size_t content_len, response_len;

	char resbuf[CFWS_MAX_RESPONSE];
	char readbuf[CFWS_MAXREAD];
	struct http_request req;

	memset(readbuf, 0, CFWS_MAXREAD);
	read(connfd, readbuf, CFWS_MAXREAD - 1);

	req = http_parse_request(readbuf);

	content_len = file_read(req.uri, &content_buf);
	if (content_len == 0) {
		const char *msg = "Could not find the specified file.";
		response_len = http_build_response(resbuf,
				HTTP_RESPONSE_NOTFOUND, msg, strlen(msg));
	} else {
		response_len = http_build_response(resbuf, HTTP_RESPONSE_OK,
				content_buf, content_len);
		free(content_buf);
	}

	write(connfd, resbuf, response_len);
	http_free_request(&req);
}
