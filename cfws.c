#include <arpa/inet.h>
#include <errno.h>
#include <linux/limits.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "http.h"

#define CFWS_MAXCONN 10   /* Max connections allowed by listen(). */
#define CFWS_MAXREAD 1024 /* Size of buffer used for reading from client. */

#define CFWS_DEFAULT_PORT 8080

size_t file_read(const char *, char **);

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

size_t file_read(const char *uri_path, char **buffer)
{
	FILE *fp;
	struct stat statbuf;
	char path[PATH_MAX];
	long len;

	/* Prepend the current working directory to the uri path */
	getcwd(path, PATH_MAX);
	strncat(path, uri_path, PATH_MAX - 1);

	/* Append 'index.html' to directory paths. */
	stat(path, &statbuf);
	if (S_ISDIR(statbuf.st_mode))
		strcat(path, "index.html");

	fp = fopen(path, "rb");
	if (fp == NULL) {
		/*
		 * File not found is a very common and harmless error, so
		 * there's no need to print it out every time.
		 */
		if (errno != ENOENT)
			perror("Failed to open file");
		return 0;
	}

	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	*buffer = malloc(len);
	fread(*buffer, 1, len, fp);

	fclose(fp);
	return len;
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
