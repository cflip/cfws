#pragma once

#include <cstdio>
#include <string>

#include "HttpRequest.h"

class CGIScript {
public:
	CGIScript(const std::string& path, const HttpRequest&);
	~CGIScript();

	bool is_open() const { return m_is_open; }

	std::string read_output();
private:
	FILE* m_pipe;
	bool m_is_open{false};
};
