#include <iostream>

#include "ClientConnection.h"
#include "HttpResponse.h"
#include "ServerConnection.h"

int main(int argc, char** argv)
{
	ServerConnection server(8080);

	std::cout << "cfws v0.1.0]\n";

	while (true) {
		std::cout << "Waiting for connections on port 8080" << std::endl;
		ClientConnection client = server.accept_client_connection();
		client.dump_request_data();

		HttpResponse http_response(ResponseCode::OK);
		http_response.add_header("Server: cfws");

		const char* message = "Welcome to the page.";
		client.send(http_response, message);
		client.close();
	}
}
