#ifndef _H_HTTP
#define _H_HTTP

#include <stddef.h>

#define CFWS_MAXURI  128
#define CFWS_MAX_RESPONSE 4096

enum http_req_method {
	HTTP_METHOD_GET
};

enum http_res_code {
	HTTP_RESPONSE_OK       = 200,
	HTTP_RESPONSE_NOTFOUND = 404
};

struct http_request {
	int method;
	char *uri;
};

struct http_request http_parse_request(const char *);
void http_free_request(struct http_request *);

int http_build_response(char *, enum http_res_code, const char *, size_t);

#endif
