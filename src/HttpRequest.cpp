#include "HttpRequest.h"

#include <iostream>

HttpRequest::HttpRequest(const std::string& request_string)
{
	size_t pos = 0;
	std::string s = request_string;
	std::string line;
	while ((pos = s.find("\r\n")) != std::string::npos) {
		line = s.substr(0, pos);

		// If the line contains a colon, we assume it's a header.
		// TODO: This may not always be the case.
		size_t delim_pos = 0;
		if ((delim_pos = line.find(":")) != std::string::npos) {
			std::string header_key = s.substr(0, delim_pos);
			std::string header_value = s.substr(delim_pos + 2, s.find("\r\n") - delim_pos - 2);
			m_headers[header_key] = header_value;
			//std::cout << "1?: {" << header_maybe << "}, 2?: {" << value_maybe << "}\n";
		}
		s.erase(0, pos + 2);
	}
}
