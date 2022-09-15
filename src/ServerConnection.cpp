#include "ServerConnection.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <strings.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netdb.h>

#include "ClientConnection.h"

static void error_and_die(const char* message)
{
	perror(message);
	exit(1);
}

ServerConnection::ServerConnection(int port)
{
	sockaddr_in address;

	if ((m_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		error_and_die("Failed to create socket");

	bzero(&address, sizeof(address));
	address.sin_family      = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port        = htons(port);

	if ((bind(m_socket_fd, (sockaddr*)&address, sizeof(address))) < 0)
		error_and_die("bind");

	if ((listen(m_socket_fd, 10)) < 0)
		error_and_die("listen");
}

ClientConnection ServerConnection::accept_client_connection()
{
	int client_socket = accept(m_socket_fd, (sockaddr*)nullptr, nullptr);
	return ClientConnection(client_socket);
}
