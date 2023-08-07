#include "file.h"

#include <errno.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define FILE_READBUF_SIZE 1024

static const char *index_names[] = { "index.html", "index.php", NULL };

const char *file_path_for_uri(const char *uri)
{
	struct stat statbuf;
	char path[PATH_MAX];
	size_t result_len;
	char *result;

	getcwd(path, PATH_MAX);
	strncat(path, uri, PATH_MAX - 1);

	/* Look for an index file if this is a directory */
	stat(path, &statbuf);
	if (S_ISDIR(statbuf.st_mode)) {
		size_t dir_index = strlen(path);
		for (int i = 0; index_names[i] != NULL; i++) {
			strcat(path, index_names[i]);
			if (access(path, F_OK) == 0) {
				/* We found an index file. */
				break;
			}
			/* Put the null terminator back where it was and try
			 * again with a different file name. */
			path[dir_index] = '\0';
		}
	}

	/* Allocate a string with only the needed size */
	result_len = strlen(path);
	result = malloc(result_len + 1);
	memcpy(result, path, result_len);
	result[result_len] = '\0';
	return result;
}

enum serve_method file_method_for_path(const char *filepath, enum http_res_code *code)
{
	struct stat statbuf;

	if (access(filepath, F_OK) != 0) {
		*code = HTTP_RESPONSE_NOTFOUND;
		return SERVE_METHOD_ERROR;
	}

	*code = HTTP_RESPONSE_OK;
	if (strstr(filepath, ".php") != 0)
		return SERVE_METHOD_PHP;

	stat(filepath, &statbuf);
	if (statbuf.st_mode & S_IXUSR) {
		return SERVE_METHOD_CGI;
	}

	return SERVE_METHOD_FILE;
}

static const char *mime_type_for_path(const char *filepath)
{
	const char *ext = strrchr(filepath, '.');
	if (ext == NULL)
		return "text/plain";
	if (strcmp(ext, ".html") == 0)
		return "text/html";
	if (strcmp(ext, ".css") == 0)
		return "text/css";
	if (strcmp(ext, ".js") == 0)
		return "text/javascript";
	if (strcmp(ext, ".png") == 0)
		return "image/png";
	if (strcmp(ext, ".gif") == 0)
		return "image/gif";
	if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0)
		return "image/jpeg";
	if (strcmp(ext, ".webp") == 0)
		return "image/webp";
	return "text/plain";
}

int file_read(const char *filepath, int sockfd)
{
	FILE *fp;
	char buffer[FILE_READBUF_SIZE];
	const char *mime_type;
	size_t bytes_read;

	fp = fopen(filepath, "rb");
	if (fp == NULL) {
		perror("Failed to open file");
		return 1;
	}

	mime_type = mime_type_for_path(filepath);
	write(sockfd, "Content-Type: ", 14);
	write(sockfd, mime_type, strlen(mime_type));
	write(sockfd, "\r\n\r\n", 4);

	while ((bytes_read = fread(buffer, 1, FILE_READBUF_SIZE, fp)) > 0)
		write(sockfd, buffer, bytes_read);

	fclose(fp);
	return 0;
}

static void cgi_setup_env(const char *filepath, const struct http_request *req)
{
	if (req->method == HTTP_METHOD_GET) {
		setenv("REQUEST_METHOD", "GET", 1);
	} else if (req->method == HTTP_METHOD_POST) {
		setenv("REQUEST_METHOD", "POST", 1);
	}

	setenv("SCRIPT_FILENAME", filepath, 1);
	if (req->query_str)
		setenv("QUERY_STRING", req->query_str, 1);

	if (req->cookie)
		setenv("HTTP_COOKIE", req->cookie, 1);

	if (req->body) {
		static char intbuf[20];
		static char *content_type = "application/x-www-form-urlencoded";
		sprintf(intbuf, "%ld", strlen(req->body));
		setenv("CONTENT_LENGTH", intbuf, 1);
		setenv("CONTENT_TYPE", content_type, strlen(content_type));
	}
}

int file_read_cgi(const char *filepath, const char *program, const struct http_request *req, int sockfd)
{
	int readfds[2];
	int writefds[2];
	pid_t pid;

	char buffer[FILE_READBUF_SIZE];
	size_t bytes_read;

	cgi_setup_env(filepath, req);

	/* Create pipes for reading from and writing to the child process. */
	if (pipe(readfds) == -1) {
		perror("read pipe");
		return 1;
	}

	if (req->body && pipe(writefds) == -1) {
		perror("write pipe");
		return 1;
	}

	pid = fork();
	if (pid == -1) {
		perror("fork");
		return 1;
	}

	if (pid == 0) {
		close(readfds[0]);
		if (readfds[1] != STDOUT_FILENO) {
			dup2(readfds[1], STDOUT_FILENO);
			close(readfds[1]);
		}

		if (req->body != NULL) {
			close(writefds[1]);
			if (writefds[0] != STDIN_FILENO) {
				dup2(writefds[0], STDIN_FILENO);
				close(writefds[0]);
			}
		}

		execl(program, program, NULL);
		/* We should only end up here if there's an error. */
		perror("exec");
		exit(1);
	}

	close(readfds[1]);
	if (req->body != NULL) {
		/* Write to stdin of child process. */
		close(writefds[0]);
		write(writefds[1], req->body, strlen(req->body) + 1);
	}

	/* Read its output from stdout. */
	while ((bytes_read = read(readfds[0], buffer, FILE_READBUF_SIZE)) > 0)
		write(sockfd, buffer, bytes_read);

	unsetenv("QUERY_STRING");
	unsetenv("HTTP_COOKIE");
	unsetenv("CONTENT");
	unsetenv("CONTENT_LENGTH");

	close(readfds[0]);
	close(writefds[1]);

	return 0;
}
