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

int net_init_server(int port)
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

	/*
	 * Set SO_REUSEADDR to allow the port to be reused, preventing errors
	 * when quickly starting and restarting the server. SO_REUSEPORT and
	 * SO_NOSIGPIPE would also be nice, but they are not supported on Linux.
	 */
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

	rc = listen(sockfd, CFWS_NET_MAXCONN);
	if (rc == -1) {
		perror("Failed to listen on server socket");
		return -1;
	}

	return sockfd;
}

struct http_request net_next_request(int serverfd, int *clientfd)
{
	int connfd;
	char readbuf[CFWS_NET_MAXREAD];
	struct http_request req;

	connfd = accept(serverfd, NULL, NULL);

	/* Read and parse the HTTP request */
	memset(readbuf, 0, CFWS_NET_MAXREAD);
	read(connfd, readbuf, CFWS_NET_MAXREAD - 1);
	req = http_parse_request(readbuf);

	*clientfd = connfd;
	return req;
}
