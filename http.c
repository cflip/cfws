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
	req.method    = HTTP_METHOD_UNKNOWN;
	req.uri       = NULL;
	req.query_str = NULL;
	req.body      = NULL;

	if (strncmp(reqstr, "GET ", 4) == 0) {
		req.method = HTTP_METHOD_GET;
		counter = reqstr + 4;
	} else if (strncmp(reqstr, "POST ", 5) == 0) {
		req.method = HTTP_METHOD_POST;
		counter = reqstr + 5;
	} else {
		fprintf(stderr, "Unhandled request type: %s\n", reqstr);
		return req;
	}

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
	
	/* TODO: Parse request headers. For now they are just ignored. */
	counter = strstr(reqstr, "\r\n\r\n");
	if (req.method == HTTP_METHOD_POST) {
		size_t body_len;
		counter += 4;

		body_len = strlen(counter);
		req.body = malloc(body_len + 1);
		memcpy(req.body, counter, body_len);
		req.body[body_len] = '\0';
	}

	return req;
}

void http_free_request(struct http_request *req)
{
	free(req->uri);
	if (req->query_str)
		free(req->query_str);
	if (req->body)
		free(req->body);
}

static const char *response_msg[] = {
	"200 OK",
	"400 Bad Request",
	"404 Not Found",
	"501 Not Implemented"
};

void http_response_statusline(enum http_res_code status_code, int sockfd)
{
	char statusline[64];
	int len;
	len = snprintf(statusline, 64, "HTTP/1.1 %s\r\n", response_msg[status_code]);
	write(sockfd, statusline, len);
}
