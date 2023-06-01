#include "file.h"

#include <errno.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define FILE_READBUF_SIZE 1024

const char *file_path_for_uri(const char *uri)
{
	struct stat statbuf;
	char path[PATH_MAX];
	size_t result_len;
	char *result;

	getcwd(path, PATH_MAX);
	strncat(path, uri, PATH_MAX - 1);

	/* Append 'index.html' if this is a directory */
	stat(path, &statbuf);
	if (S_ISDIR(statbuf.st_mode))
		strcat(path, "index.html");

	/* Allocate a string with only the needed size */
	result_len = strlen(path);
	result = malloc(result_len + 1);
	memcpy(result, path, result_len);
	result[result_len] = '\0';
	return result;
}

enum serve_method file_method_for_path(const char *filepath, enum http_res_code *code)
{
	if (access(filepath, F_OK) != 0) {
		*code = HTTP_RESPONSE_NOTFOUND;
		return SERVE_METHOD_ERROR;
	}

	*code = HTTP_RESPONSE_OK;
	if (strstr(filepath, ".php") != 0)
		return SERVE_METHOD_PHP;

	return SERVE_METHOD_FILE;
}

int file_read(const char *filepath, int sockfd)
{
	FILE *fp;
	char buffer[FILE_READBUF_SIZE];
	size_t bytes_read;

	const char *content_type = "Content-Type: text/html\r\n\r\n";

	fp = fopen(filepath, "rb");
	if (fp == NULL) {
		perror("Failed to open file");
		return 1;
	}

	write(sockfd, content_type, strlen(content_type));
	while ((bytes_read = fread(buffer, 1, FILE_READBUF_SIZE, fp)) > 0)
		write(sockfd, buffer, bytes_read);

	fclose(fp);
	return 0;
}

int file_read_php(const char *filepath, const char *query_str, int sockfd)
{
	FILE *fp;
	char buffer[FILE_READBUF_SIZE];
	size_t bytes_read;

	setenv("REQUEST_METHOD", "GET", 1);
	setenv("SCRIPT_FILENAME", filepath, 1);
	if (query_str)
		setenv("QUERY_STRING", query_str, 1);

	fp = popen("php-cgi", "r");
	if (fp == NULL) {
		perror("Failed to read command");
		return 1;
	}

	while ((bytes_read = fread(buffer, 1, FILE_READBUF_SIZE, fp)) > 0)
		write(sockfd, buffer, bytes_read);

	pclose(fp);
	return 0;
}
