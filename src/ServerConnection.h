#pragma once

class ClientConnection;

class ServerConnection {
public:
	ServerConnection(int port);

	ClientConnection accept_client_connection();
private:
	int m_socket_fd;
};
