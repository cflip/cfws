#include "ClientConnection.h"

#include <unistd.h>
#include <iostream>
#include <sstream>

ClientConnection::ClientConnection(int socket)
	: m_socket_fd(socket)
{
}

void ClientConnection::dump_request_data()
{
	constexpr int BUFFER_SIZE = 4096;
	char buffer[BUFFER_SIZE+1];
	int n;

	memset(buffer, 0, BUFFER_SIZE);
	while ((n = read(m_socket_fd, buffer, BUFFER_SIZE-1)) > 0) {
		std::cout << buffer;

		if (buffer[n-1] == '\n')
			break;

		memset(buffer, 0, BUFFER_SIZE);
	}
}

bool ClientConnection::send(const HttpResponse& response, const char* message)
{
	if (!m_is_open)
		return false;

	std::stringstream ss;
	ss << response.to_string()
	   << "\r\n\r\n"
	   << message;

	std::string result = ss.str();
	write(m_socket_fd, result.c_str(), result.length());

	return true;
}

void ClientConnection::close()
{
	m_is_open = false;
	::close(m_socket_fd);
}
