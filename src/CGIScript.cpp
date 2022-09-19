#include "CGIScript.h"

#include <cstdlib>
#include <string>
#include <sstream>

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

	for (auto key : m_environment_variables)
		unsetenv(key);
}

void CGIScript::set_environment(const char* key, const char* value)
{
	m_environment_variables.push_back(key);
	setenv(key, value, true);
}

bool CGIScript::open()
{
	m_pipe = popen(m_script_path.c_str(), "r");
	if (!m_pipe) {
		perror("cfws: popen");
		return false;
	}

	m_is_open = true;
	return true;
}

std::string CGIScript::read_output()
{
	std::stringstream sstream;

	char ch;
	while ((ch = fgetc(m_pipe)) != EOF)
		sstream << ch;

	return sstream.str();
}
