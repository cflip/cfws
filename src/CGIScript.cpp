#include "CGIScript.h"

#include <cstdlib>
#include <string>
#include <sstream>

CGIScript::CGIScript(const std::string& path, const HttpRequest& request)
{
	setenv("CONTENT_LENGTH", "0", true);
	setenv("REQUEST_URI", request.uri().c_str(), true);
	setenv("SCRIPT_NAME", path.c_str(), true);
	setenv("SCRIPT_FILENAME", path.c_str(), true);
	setenv("REQUEST_METHOD", "GET", true);
	setenv("SERVER_PROTOCOL", "HTTP/1.1", true);
	setenv("SERVER_SOFTWARE", "cfws/1.0-dev", true);

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

	unsetenv("CONTENT_LENGTH");
	unsetenv("REQUEST_URI");
	unsetenv("SCRIPT_NAME");
	unsetenv("SCRIPT_FILENAME");
	unsetenv("REQUEST_METHOD");
	unsetenv("SERVER_PROTOCOL");
	unsetenv("SERVER_SOFTWARE");
}

std::string CGIScript::read_output()
{
	std::stringstream sstream;

	char ch;
	while ((ch = fgetc(m_pipe)) != EOF)
		sstream << ch;

	return sstream.str();
}
