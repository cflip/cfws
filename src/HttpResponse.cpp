#include "HttpResponse.h"

#include <sstream>

void HttpResponse::add_headers_and_content(const std::string& input)
{
	bool is_parsing_headers = true;

	size_t pos = 0;
	std::string s = input;
	std::string line;
	while ((pos = s.find("\n")) != std::string::npos) {
		line = s.substr(0, pos + 1);

		if (is_parsing_headers) {
			size_t delim_pos = 0;
			if ((delim_pos = line.find(":")) != std::string::npos) {
				std::string header_key = s.substr(0, delim_pos);
				std::string header_value = s.substr(delim_pos + 2, s.find("\n") - delim_pos - 2);
				m_headers[header_key] = header_value;
			} else {
				is_parsing_headers = false;
			}
		} else {
			m_content += line;
		}
		s.erase(0, pos + 1);
	}
}

static std::string status_code_string(HttpStatusCode status_code)
{
	switch (status_code) {
	case HttpStatusCode::OK:
		return "200 OK";
	case HttpStatusCode::Forbidden:
		return "403 Forbidden";
	case HttpStatusCode::NotFound:
		return "404 Not Found";
	case HttpStatusCode::InternalServerError:
		return "500 Internal Server Error";
	}
}

std::string HttpResponse::to_string() const
{
	std::stringstream string_stream;
	string_stream << "HTTP/1.0 " << status_code_string(m_status_code) << "\r\n";
	for (const auto& header : m_headers)
		string_stream << header.first << ": " << header.second << "\r\n";
	string_stream << "\r\n"
				  << m_content;

	return string_stream.str();
}
