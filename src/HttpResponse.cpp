#include "HttpResponse.h"

#include <sstream>

static std::string status_code_string(HttpStatusCode status_code)
{
	switch (status_code) {
	case HttpStatusCode::OK:
		return "200 OK";
	}
}

HttpResponse::HttpResponse(HttpStatusCode status_code)
	: m_status_code(status_code)
{
}

void HttpResponse::add_header(const std::string& header, const std::string& value)
{
	m_headers[header] = value;
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
