#include "CGIScript.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>

CGIScript::CGIScript(const std::string& script_path)
	: m_script_path(script_path)
{
	set_environment("SCRIPT_NAME", script_path.c_str());
	set_environment("SCRIPT_FILENAME", script_path.c_str());
	set_environment("SERVER_PROTOCOL", "HTTP/1.1");
	set_environment("SERVER_SOFTWARE", "cfws");
}

CGIScript::~CGIScript()
{
	pclose(m_pipe);

	for (const auto* key : m_environment_variables)
		unsetenv(key);
}

void CGIScript::set_environment(const char* key, const char* value)
{
	m_environment_variables.push_back(key);
	setenv(key, value, 1);
}

bool CGIScript::open()
{
	m_pipe = popen(m_script_path.c_str(), "r");
	if (m_pipe == nullptr) {
		perror("cfws: popen");
		return false;
	}

	m_is_open = true;
	return true;
}

std::string CGIScript::read_output()
{
	std::stringstream sstream;

	char ch = 0;
	while ((ch = fgetc(m_pipe)) != EOF)
		sstream << ch;

	return sstream.str();
}

void CGIScript::validate_path(const std::string& script_path)
{
	namespace fs = std::filesystem;

	if (!fs::exists(script_path)) {
		std::cerr << "cfws: Script not found: " << script_path << std::endl;
		exit(1);
	}

	if (access(script_path.c_str(), X_OK)) {
		std::cerr << "cfws: Script does not have execute permissions: " << script_path << std::endl;
		exit(1);
	}
}
