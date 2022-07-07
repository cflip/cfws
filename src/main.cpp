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

		HttpResponse response;
		response.add_header("Server", "cfws");
		if (request.header("Accept-Language").find("zh") == std::string::npos) {
			response.set_status_code(HttpStatusCode::Forbidden);
			response.set_content("Please set your language to Chinese to access this page.");
		} else {
			response.set_status_code(HttpStatusCode::OK);
			response.set_content("Welcome to the page.");
		}

		client.send(response);
		client.close_connection();
	}
}
