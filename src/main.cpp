#include <getopt.h>

#include <iostream>
#include <filesystem>
#include <fstream>

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

static HttpResponse serve_from_cgi(const std::string& executable_path, HttpRequest request)
{
	HttpResponse response;
	response.add_header("Server", "cfws");

	setenv("CONTENT_LENGTH", "0", true);
	setenv("REQUEST_URI", request.uri().c_str(), true);
	setenv("SCRIPT_NAME", executable_path.c_str(), true);
	setenv("SCRIPT_FILENAME", executable_path.c_str(), true);
	setenv("REQUEST_METHOD", "GET", true);
	setenv("SERVER_PROTOCOL", "HTTP/1.1", true);
	setenv("SERVER_SOFTWARE", "cfws/1.0-dev", true);

	std::stringstream sstream;

	FILE* fp = popen(executable_path.c_str(), "r");
	if (!fp) {
		perror("cfws: popen");
		response.set_status_code(HttpStatusCode::InternalServerError);
		response.add_header("Content-Type", "text/plain");
		response.set_content("Failed to open CGI executable!");
		return response;
	}

	char ch;
	while ((ch = fgetc(fp)) != EOF)
		sstream << ch;

	pclose(fp);

	unsetenv("CONTENT_LENGTH");
	unsetenv("REQUEST_URI");
	unsetenv("SCRIPT_NAME");
	unsetenv("SCRIPT_FILENAME");
	unsetenv("REQUEST_METHOD");
	unsetenv("SERVER_PROTOCOL");
	unsetenv("SERVER_SOFTWARE");

	// TODO: We should be able to construct a repsonse from an entire string
	//       instead of always needing to individually set headers and content.
	response.set_status_code(HttpStatusCode::OK);
	response.add_headers_and_content(sstream.str());
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
	std::cout << "Serving a " << (in_cgi_mode ? "CGI exectuable" : "directory") << " on port " << port << std::endl;

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
