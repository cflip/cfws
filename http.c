#include "http.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct http_request http_parse_request(const char *reqstr)
{
	struct http_request req;
	char uribuf[CFWS_MAXURI];
	size_t urilen;

	sscanf(reqstr, "GET %s HTTP/1.1", uribuf);

	/* TODO: Support other method types, notably POST */
	req.method = HTTP_METHOD_GET;

	urilen = strlen(uribuf);
	req.uri = malloc(urilen + 1);
	memcpy(req.uri, uribuf, urilen + 1);
	
	/* TODO: Parse request headers */

	return req;
}

void http_free_request(struct http_request *req)
{
	free(req->uri);
}

static const char *response_msg[2] = {
	"200 OK",
	"404 Not Found"
};

void http_response_statusline(enum http_res_code status_code, int sockfd)
{
	char statusline[64];
	int len;
	len = snprintf(statusline, 64, "HTTP/1.1 %s\r\n", response_msg[status_code]);
	write(sockfd, statusline, len);
}
