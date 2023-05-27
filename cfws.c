#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "net.h"

#define CFWS_DEFAULT_PORT 8080

int main(int argc, char *argv[])
{
	int port = CFWS_DEFAULT_PORT;
	int serverfd, clientfd;

	serverfd = initialize_server(port);
	if (serverfd == -1)
		return 1;

	printf("Serving a directory at localhost:%d\n", port);

	while (1) {
		clientfd = accept(serverfd, NULL, NULL);
		handle_connection(clientfd);
		close(clientfd);
	}

	close(serverfd);
	return 0;
}
