#pragma once

#include <map>
#include <string>

enum class HttpStatusCode {
	OK = 200,
	Forbidden = 403,
	NotFound = 404
};

class HttpResponse {
public:
	void add_header(const std::string& header, const std::string& value)
	{
		m_headers[header] = value;
	}

	void set_status_code(HttpStatusCode status_code) { m_status_code = status_code; }
	void set_content(const std::string& content) { m_content = content; }

	std::string to_string() const;
private:
	HttpStatusCode m_status_code;
	std::map<std::string, std::string> m_headers;
	std::string m_content;
};
