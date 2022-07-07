#pragma once

#include <map>
#include <string>

enum class HttpStatusCode {
	OK = 200
};

class HttpResponse {
public:
	HttpResponse(HttpStatusCode status_code);

	void add_header(const std::string& header, const std::string& value);
	void set_content(const std::string& content) { m_content = content; }

	std::string to_string() const;
private:
	HttpStatusCode m_status_code;
	std::map<std::string, std::string> m_headers;
	std::string m_content;
};
