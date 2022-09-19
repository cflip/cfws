#pragma once

#include <cstdio>
#include <string>
#include <vector>

class CGIScript {
public:
	CGIScript(const std::string& script_path);
	~CGIScript();

	void set_environment(const char* key, const char* value);
	bool open();

	std::string read_output();
private:
	FILE* m_pipe;
	const std::string& m_script_path;
	bool m_is_open{false};

	std::vector<const char*> m_environment_variables;
};
