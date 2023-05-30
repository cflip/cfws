#ifndef _H_FILE
#define _H_FILE

#include <stddef.h>

#include "http.h"

int file_handle_request(struct http_request *, int);

int file_read(const char *, int);
int file_read_cgi(const char *, int);

#endif
