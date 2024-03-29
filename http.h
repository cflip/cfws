#ifndef _H_HTTP
#define _H_HTTP

#include <stddef.h>

#define CFWS_MAXURI  128
#define CFWS_MAX_RESPONSE 4096

enum http_req_method {
	HTTP_METHOD_UNKNOWN,
	HTTP_METHOD_GET,
	HTTP_METHOD_POST
};

enum http_res_code {
	HTTP_RESPONSE_OK,
	HTTP_RESPONSE_BADREQUEST,
	HTTP_RESPONSE_NOTFOUND,
	HTTP_RESPONSE_NOTIMPLEMENTED
};

struct http_request {
	int method;
	char *uri;
	char *query_str;
	char *cookie;
	char *body;
};

struct http_request http_parse_request(const char *);
void http_free_request(struct http_request *);

void http_response_statusline(enum http_res_code, int);

#endif
