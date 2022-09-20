#pragma once

class ClientConnection;

class ServerConnection {
public:
	ServerConnection(int port);

	ClientConnection accept_client_connection() const;

private:
	int m_socket_fd;
};
