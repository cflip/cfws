#include "HttpResponse.h"

#include <sstream>

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
	string_stream << "\r\n" << m_content;

	return string_stream.str();
}
