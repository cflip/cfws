#pragma once

#include "HttpRequest.h"
#include "HttpResponse.h"

class ClientConnection {
public:
	ClientConnection(int socket);

	HttpRequest read_request() const;

	bool send(const HttpResponse&) const;
	void close_connection();

private:
	int m_socket_fd;
	bool m_is_open { true };
};
