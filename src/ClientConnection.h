#pragma once

#include "HttpRequest.h"
#include "HttpResponse.h"

class ClientConnection {
public:
	ClientConnection(int socket);

	HttpRequest read_request();

	bool send(const HttpResponse&);
	void close_connection();
private:
	int m_socket_fd;
	bool m_is_open { true };
};
