#include "ServerConnection.h"

#include <sys/types.h>

#ifdef _WIN32
	#include <winsock2.h>
	#pragma comment(lib, "ws2_32.lib")
#else
	#include <arpa/inet.h>
	#include <sys/ioctl.h>
	#include <sys/socket.h>
	#include <sys/time.h>
	#include <unistd.h>
#endif

#include "ClientConnection.h"

static void error_and_die(const char* message)
{
	perror(message);
	exit(1);
}

ServerConnection::ServerConnection(int port)
{
#ifdef _WIN32
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		error_and_die("Failed to initialize Winsock");
#endif

	sockaddr_in address {};
	int socket_options = 1;

	if ((m_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		error_and_die("Failed to create socket");

#ifndef _WIN32
	if (setsockopt(m_socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &socket_options, sizeof(socket_options)) != 0)
		error_and_die("setsockopt");
#endif

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(port);

	if ((bind(m_socket_fd, (sockaddr*)&address, sizeof(address))) < 0)
		error_and_die("bind");

	if ((listen(m_socket_fd, 10)) < 0)
		error_and_die("listen");
}

ClientConnection ServerConnection::accept_client_connection() const
{
	int client_socket = accept(m_socket_fd, (sockaddr*)nullptr, nullptr);
	return { client_socket };
}
