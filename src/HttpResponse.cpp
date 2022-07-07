#include "HttpResponse.h"

HttpResponse::HttpResponse(ResponseCode response_code)
{
	m_string_stream << "HTTP/1.0 200 OK";
}

void HttpResponse::add_header(std::string header)
{
	m_string_stream << '\n' << header;
}

std::string HttpResponse::to_string() const
{
	return m_string_stream.str();
}
