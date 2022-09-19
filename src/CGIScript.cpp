#include "CGIScript.h"

#include <cstdlib>
#include <string>
#include <sstream>

CGIScript::CGIScript(const std::string& path, const HttpRequest& request)
{
	set_environment("CONTENT_LENGTH", "0");
	set_environment("REQUEST_URI", request.uri().c_str());
	set_environment("PATH_INFO", request.uri().c_str());
	set_environment("SCRIPT_NAME", path.c_str());
	set_environment("SCRIPT_FILENAME", path.c_str());
	set_environment("REQUEST_METHOD", "GET");
	set_environment("SERVER_PROTOCOL", "HTTP/1.1");
	set_environment("SERVER_SOFTWARE", "cfws/1.0-dev");

	m_pipe = popen(path.c_str(), "r");
	if (!m_pipe) {
		perror("cfws: popen");
		return;
	}

	m_is_open = true;
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

std::string CGIScript::read_output()
{
	std::stringstream sstream;

	char ch;
	while ((ch = fgetc(m_pipe)) != EOF)
		sstream << ch;

	return sstream.str();
}
