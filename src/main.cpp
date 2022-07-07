#include <iostream>

#include "ClientConnection.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "ServerConnection.h"

int main(int argc, char** argv)
{
	ServerConnection server(8080);

	std::cout << "cfws v0.1.0]\n";

	while (true) {
		std::cout << "Waiting for connections on port 8080" << std::endl;
		ClientConnection client = server.accept_client_connection();
		HttpRequest request = client.read_request();

		HttpResponse response(HttpStatusCode::OK);
		response.add_header("Server", "cfws");
		response.set_content("Welcome to the page.");

		client.send(response);
		client.close_connection();
	}
}
