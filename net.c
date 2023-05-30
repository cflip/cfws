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
	const char *header = "HTTP/1.1 200 OK\r\n";
	char readbuf[CFWS_MAXREAD];
	struct http_request req;

	memset(readbuf, 0, CFWS_MAXREAD);
	read(connfd, readbuf, CFWS_MAXREAD - 1);

	req = http_parse_request(readbuf);

	write(connfd, header, strlen(header));
	if (file_handle_request(&req, connfd) != 0) {
		const char *msg = "\r\nCould not find the specified file.";
		write(connfd, msg, strlen(msg));
	}

	http_free_request(&req);
}
