#include "file.h"

#include <errno.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

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
	char ch;

	fp = fopen(filepath, "rb");
	if (fp == NULL) {
		perror("Failed to open file");
		return 1;
	}

	write(sockfd, "Content-Type: text/html\r\n\r\n", 27);
	/* TODO: Implement a buffered read from FILE* function */
	while ((ch = fgetc(fp)) != EOF) {
		write(sockfd, &ch, 1);
	}

	fclose(fp);
	return 0;
}

int file_read_php(const char *filepath, int sockfd)
{
	FILE *fp;
	char cmdbuf[PATH_MAX];
	char ch;

	strcpy(cmdbuf, "php-cgi ");
	strcat(cmdbuf, filepath);

	printf("r %s\n", cmdbuf);

	fp = popen(cmdbuf, "r");
	if (fp == NULL) {
		perror("Failed to read command");
		return 1;
	}

	/* TODO: Implement a buffered read from FILE* function */
	while ((ch = fgetc(fp)) != EOF) {
		write(sockfd, &ch, 1);
	}

	pclose(fp);
	return 0;
}
