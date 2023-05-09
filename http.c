#include "http.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void http_build_response(char **res, enum http_res_code code, const char *msg)
{
	*res = malloc(128);
	sprintf(*res, "HTTP/1.1 200 OK\r\n\r\n%s\r\n", msg);
}
