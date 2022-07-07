#pragma once

#include <sstream>
#include <string>

enum class ResponseCode {
	OK = 200
};

class HttpResponse {
public:
	HttpResponse(ResponseCode);

	void add_header(std::string header);
	std::string to_string() const;
private:
	std::stringstream m_string_stream;
};
