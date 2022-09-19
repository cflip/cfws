#pragma once

#include <cstdio>
#include <string>
#include <vector>

#include "HttpRequest.h"

class CGIScript {
public:
	CGIScript(const std::string& path, const HttpRequest&);
	~CGIScript();

	void set_environment(const char* key, const char* value);
	bool is_open() const { return m_is_open; }

	std::string read_output();
private:
	FILE* m_pipe;
	bool m_is_open{false};

	std::vector<const char*> m_environment_variables;
};
