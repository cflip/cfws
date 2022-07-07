#pragma once

#include <map>
#include <string>

class HttpRequest {
public:
	HttpRequest(const std::string& request_string);

	std::string uri() const { return m_uri; }
	std::string header(const std::string& header_key) const { return m_headers.at(header_key); };
private:
	std::map<std::string, std::string> m_headers;
	std::string m_uri;
};
