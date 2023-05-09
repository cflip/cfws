#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "http.h"

#define CFWS_MAXCONN 10   /* Max connections allowed by listen(). */
#define CFWS_MAXREAD 1024 /* Size of buffer used for reading from client. */

#define CFWS_DEFAULT_PORT 8080

int  initialize_server(int);
void handle_connection();

int main(int argc, char *argv[])
{
	int port = CFWS_DEFAULT_PORT;
	int serverfd, clientfd;

	serverfd = initialize_server(port);
	if (serverfd == -1)
		return 1;

	printf("Serving a directory at localhost:%d\n", port);

	while (1) {
		clientfd = accept(serverfd, NULL, NULL);
		handle_connection(clientfd);
		close(clientfd);
	}

	close(serverfd);
	return 0;
}

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
	char msgbuf[128];
	char *resbuf;
	char readbuf[CFWS_MAXREAD];
	struct http_request req;

	memset(readbuf, 0, CFWS_MAXREAD);
	read(connfd, readbuf, CFWS_MAXREAD - 1);

	req = http_parse_request(readbuf);

	snprintf(msgbuf, 128, "Welcome to %s", req.uri);

	http_build_response(&resbuf, HTTP_RESPONSE_OK, msgbuf);
	write(connfd, resbuf, strlen(resbuf));

	free(resbuf);
	http_free_request(&req);
}

