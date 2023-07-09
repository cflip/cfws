#ifndef _H_NET
#define _H_NET

#define CFWS_NET_MAXCONN 10   /* Max connections allowed by listen(). */
#define CFWS_NET_MAXREAD 1024 /* Size of buffer used for reading from client. */

#include "http.h"

int net_init_server(int);
int net_next_request(int serverfd, int *clientfd, struct http_request *);

#endif
