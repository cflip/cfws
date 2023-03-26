#ifndef _WIN32
#include <getopt.h>
#endif

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#ifndef _WIN32
#include "CGIScript.h"
#endif
#include "ClientConnection.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "ServerConnection.h"

static std::string content_type_for_extension(const std::filesystem::path& extension)
{
	if (extension == ".html")
		return "text/html";
	if (extension == ".css")
		return "text/css";
	if (extension == ".gif")
		return "image/gif";
	if (extension == ".png")
		return "image/png";
	if (extension == ".webp")
		return "image/webp";
	return "text/plain";
}

static HttpResponse serve_from_filesystem(const HttpRequest& request)
{
	namespace fs = std::filesystem;

	HttpResponse response;
	response.add_header("Server", "cfws");

	// Remove leading slash from the path if it exists
	std::string relative_request_path = request.uri();
	while (relative_request_path.length() > 0 && *relative_request_path.begin() == '/')
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
		response.add_header("Content-Type", content_type_for_extension(request_path.extension()));
		response.set_content(file_contents);
	} else {
		response.set_status_code(HttpStatusCode::NotFound);
		response.add_header("Content-Type", "text/plain");
		response.set_content("Page not found!");
	}

	return response;
}

#ifndef _WIN32
static HttpResponse serve_from_cgi(const std::string& script_path, const HttpRequest& request)
{
	HttpResponse response;
	response.add_header("Server", "cfws");

	// Split URI between the path and the query parameter string
	std::stringstream uri_stream(request.uri());
	std::string segment;
	std::vector<std::string> segment_list;

	while (std::getline(uri_stream, segment, '?')) {
		segment_list.push_back(segment);
	}

	CGIScript script(script_path);
	script.set_environment("REQUEST_METHOD", "GET");
	script.set_environment("REQUEST_URI", request.uri().c_str());
	script.set_environment("PATH_INFO", segment_list[0].c_str());
	if (segment_list.size() > 1)
		script.set_environment("QUERY_STRING", segment_list[1].c_str());
	script.set_environment("CONTENT_LENGTH", "0");

	if (!script.open()) {
		response.set_status_code(HttpStatusCode::InternalServerError);
		response.add_header("Content-Type", "text/plain");
		response.set_content("Failed to open CGI script!");
		return response;
	}

	response.set_status_code(HttpStatusCode::OK);
	response.add_headers_and_content(script.read_output());
	return response;
}
#endif

#ifndef _WIN32
static option long_options[] = {
	{ "cgi", required_argument, nullptr, 'c' },
	{ "port", required_argument, nullptr, 'p' },
};
#endif

int main(int argc, char** argv)
{
	int port = 8080;
	bool in_cgi_mode = false;
	std::string cgi_program_name;

	// TODO: getopt.h is not available on Windows, so this part needs to be rewritten.
#ifndef _WIN32
	int c = 0;
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

	// Check the script path to ensure that it is a valid executable
	// script before attempting to start the server.
	if (in_cgi_mode)
		CGIScript::validate_path(cgi_program_name);
#endif

	ServerConnection server(port);
	std::cout << "Serving a " << (in_cgi_mode ? "CGI script" : "directory") << " on port " << port << std::endl;

	while (true) {
		ClientConnection client = server.accept_client_connection();
		HttpRequest request = client.read_request();
		HttpResponse response;

		// TODO: Support running CGI executables on Windows
#ifndef _WIN32
		if (in_cgi_mode) {
			response = serve_from_cgi(cgi_program_name, request);
		} else
#endif
			response = serve_from_filesystem(request);


		client.send(response);
		client.close_connection();
	}
}
