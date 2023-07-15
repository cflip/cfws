#ifndef _H_FILE
#define _H_FILE

#include <stddef.h>

#include "http.h"

enum serve_method {
	SERVE_METHOD_FILE,
	SERVE_METHOD_PHP,
	SERVE_METHOD_CGI,
	SERVE_METHOD_ERROR
};

const char *file_path_for_uri(const char *);

enum serve_method file_method_for_path(const char *, enum http_res_code *);

int file_read(const char *, int);
int file_read_cgi(const char *, const char *, const struct http_request *, int);

#endif
