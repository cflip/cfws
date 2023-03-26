#include "ClientConnection.h"

#include <cstring>
#include <iostream>
#include <sstream>

#ifdef _WIN32
	#include <winsock2.h>
	#pragma comment(lib, "ws2_32.lib")
#else
	#include <unistd.h>
#endif

ClientConnection::ClientConnection(int socket)
	: m_socket_fd(socket)
{
}

HttpRequest ClientConnection::read_request() const
{
	// TODO: Clean up this code to ensure it works with multiple lines
	//       and not risk a buffer overflow.
	constexpr int BUFFER_SIZE = 4096;
	char buffer[BUFFER_SIZE + 1];
	int n = 0;

	memset(buffer, 0, BUFFER_SIZE);
#ifdef _WIN32
	n = recv(m_socket_fd, buffer, BUFFER_SIZE - 1, 0);
#else
	n = read(m_socket_fd, buffer, BUFFER_SIZE - 1);
#endif
	while (n > 0) {
		if (buffer[n - 1] == '\n')
			break;

		memset(buffer, 0, BUFFER_SIZE);
	}

	return HttpRequest(buffer);
}

bool ClientConnection::send(const HttpResponse& response) const
{
	if (!m_is_open)
		return false;

	std::string result = response.to_string();
#ifdef _WIN32
	::send(m_socket_fd, result.c_str(), result.length(), 0);
#else
	write(m_socket_fd, result.c_str(), result.length());
#endif

	return true;
}

void ClientConnection::close_connection()
{
	m_is_open = false;
#ifdef _WIN32
	closesocket(m_socket_fd);
#else
	close(m_socket_fd);
#endif
}
