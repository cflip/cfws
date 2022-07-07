#include "ClientConnection.h"

#include <unistd.h>
#include <iostream>
#include <sstream>

ClientConnection::ClientConnection(int socket)
	: m_socket_fd(socket)
{
}

HttpRequest ClientConnection::read_request()
{
	// TODO: Clean up this code to ensure it works with multiple lines
	//       and not risk a buffer overflow.
	constexpr int BUFFER_SIZE = 4096;
	char buffer[BUFFER_SIZE+1];
	int n;

	memset(buffer, 0, BUFFER_SIZE);
	while ((n = read(m_socket_fd, buffer, BUFFER_SIZE-1)) > 0) {
		if (buffer[n-1] == '\n')
			break;

		memset(buffer, 0, BUFFER_SIZE);
	}

	return HttpRequest(buffer);
}

bool ClientConnection::send(const HttpResponse& response)
{
	if (!m_is_open)
		return false;

	std::string result = response.to_string();
	write(m_socket_fd, result.c_str(), result.length());

	return true;
}

void ClientConnection::close_connection()
{
	m_is_open = false;
	close(m_socket_fd);
}
