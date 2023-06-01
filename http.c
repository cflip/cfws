#include "http.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct http_request http_parse_request(const char *reqstr)
{
	char uribuf[CFWS_MAXURI];
	size_t urilen = 0, query_len = 0;
	const char *counter;

	struct http_request req;
	req.method    = HTTP_METHOD_GET;
	req.uri       = NULL;
	req.query_str = NULL;

	/* TODO: Support other request methods such as POST */
	if (strncmp(reqstr, "GET ", 4) != 0) {
		fprintf(stderr, "Unhandled request type: %s\n", reqstr);
		return req;
	}

	counter = reqstr + 4;
	while (*counter != ' ' && *counter != '?' && *counter != 0
			&& urilen < CFWS_MAXURI) {
		uribuf[urilen++] = *counter;
		++counter;
	}

	req.uri = malloc(urilen + 1);
	memcpy(req.uri, uribuf, urilen);
	req.uri[urilen] = '\0';

	/* Parse the query string if one exists. */
	if (*counter == '?') {
		/* Skip the question mark at the beginning. */
		counter++;

		while (*counter != ' ' && *counter != 0
				&& query_len < CFWS_MAXURI) {
			uribuf[query_len++] = *counter;
			++counter;
		}

		req.query_str = malloc(query_len + 1);
		memcpy(req.query_str, uribuf, query_len);
		req.query_str[query_len] = '\0';
	}
	
	/* TODO: Parse request headers */

	return req;
}

void http_free_request(struct http_request *req)
{
	free(req->uri);
	if (req->query_str)
		free(req->query_str);
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
