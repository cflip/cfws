#ifndef _H_NET
#define _H_NET

#define CFWS_MAXCONN 10   /* Max connections allowed by listen(). */
#define CFWS_MAXREAD 1024 /* Size of buffer used for reading from client. */

int  initialize_server(int);
void handle_connection(int);

#endif
