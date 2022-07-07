#pragma once

#include "HttpResponse.h"

class ClientConnection {
public:
	ClientConnection(int socket);

	void dump_request_data();

	bool send(const HttpResponse&);
	void close_connection();
private:
	int m_socket_fd;
	bool m_is_open { true };
};
