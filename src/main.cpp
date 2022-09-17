#include <getopt.h>

#include <iostream>
#include <filesystem>
#include <fstream>

#include "CGIScript.h"
#include "ClientConnection.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "ServerConnection.h"

static HttpResponse serve_from_filesystem(HttpRequest request)
{
	namespace fs = std::filesystem;

	HttpResponse response;
	response.add_header("Server", "cfws");

	// Remove leading slash from the path if it exists
	std::string relative_request_path = request.uri();
	while (*relative_request_path.begin() == '/')
		relative_request_path.erase(0, 1);

	fs::path request_path = fs::current_path() / relative_request_path;

	// Look for an index.html if the requested path is a directory
	if (fs::is_directory(request_path)) {
		request_path /= "index.html";
	}

	if (fs::exists(request_path)) {
		std::string file_path = request_path.string();
		std::ifstream input_file(file_path);
		if (!input_file.is_open()) {
			std::cerr << "Failed to open file " << file_path << std::endl;
		}

		std::string file_contents((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());

		response.set_status_code(HttpStatusCode::OK);
		response.add_header("Content-Type", "text/plain");
		response.set_content(file_contents);
	} else {
		response.set_status_code(HttpStatusCode::NotFound);
		response.add_header("Content-Type", "text/plain");
		response.set_content("Page not found!");
	}

	return response;
}

static HttpResponse serve_from_cgi(const std::string& script_path, HttpRequest request)
{
	HttpResponse response;
	response.add_header("Server", "cfws");

	CGIScript script(script_path, request);
	if (!script.is_open()) {
		response.set_status_code(HttpStatusCode::InternalServerError);
		response.add_header("Content-Type", "text/plain");
		response.set_content("Failed to open CGI script!");
		return response;
	}

	response.set_status_code(HttpStatusCode::OK);
	response.add_headers_and_content(script.read_output());
	return response;
}

static option long_options[] = {
	{ "cgi", required_argument, NULL, 'c' },
	{ "port", required_argument, NULL, 'p' },
};

int main(int argc, char** argv)
{
	int port = 8080;
	bool in_cgi_mode = false;
	std::string cgi_program_name;

	int c;
	int option_index = 0;
	while ((c = getopt_long(argc, argv, "c:p:", long_options, &option_index)) != -1) {
		switch (c) {
		case 'c':
			in_cgi_mode = true;
			cgi_program_name = optarg;
			break;
		case 'p':
			port = atoi(optarg);
			if (port == 0) {
				std::cerr << "cfws: Specified port is not a valid number" << std::endl;
				exit(1);
			}
			break;
		default:
			break;
		}
	}

	ServerConnection server(port);
	std::cout << "Serving a " << (in_cgi_mode ? "CGI script" : "directory") << " on port " << port << std::endl;

	while (true) {
		ClientConnection client = server.accept_client_connection();
		HttpRequest request = client.read_request();
		HttpResponse response;

		if (in_cgi_mode) {
			response = serve_from_cgi(cgi_program_name, request);
		} else {
			response = serve_from_filesystem(request);
		}

		client.send(response);
		client.close_connection();
	}
}
