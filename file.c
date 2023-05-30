#include "file.h"

#include <errno.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int file_handle_request(struct http_request *req, int sockfd)
{
	if (strstr(req->uri, ".php") != 0) {
		return file_read_cgi(req->uri, sockfd);
	}
	return file_read(req->uri, sockfd);
}

int file_read(const char *uri_path, int sockfd)
{
	/* TODO: Implement this again */
	return 1;

	FILE *fp;
	struct stat statbuf;
	char path[PATH_MAX];
	long len;

	/* Prepend the current working directory to the uri path */
	getcwd(path, PATH_MAX);
	strncat(path, uri_path, PATH_MAX - 1);

	/* Append 'index.html' to directory paths. */
	stat(path, &statbuf);
	if (S_ISDIR(statbuf.st_mode))
		strcat(path, "index.html");

	fp = fopen(path, "rb");
	if (fp == NULL) {
		/*
		 * File not found is a very common and harmless error, so
		 * there's no need to print it out every time.
		 */
		if (errno != ENOENT)
			perror("Failed to open file");
		return 0;
	}

	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	/*buffer = malloc(len);*/
	/*fread(*buffer, 1, len, fp);*/

	fclose(fp);
	return len;
}

int file_read_cgi(const char *uri_path, int sockfd)
{
	/* TODO: Lazy copy paste just to get it working */
	FILE *fp;
	struct stat statbuf;
	char cmdbuf[PATH_MAX];
	char path[PATH_MAX];
	char ch;

	/* Prepend the current working directory to the uri path */
	getcwd(path, PATH_MAX);
	strncat(path, uri_path, PATH_MAX - 1);

	/* Append 'index.php' to directory paths. */
	stat(path, &statbuf);
	if (S_ISDIR(statbuf.st_mode))
		strcat(path, "index.php");

	strcpy(cmdbuf, "php-cgi ");
	strcat(cmdbuf, path);

	fp = popen(cmdbuf, "r");
	if (fp == NULL) {
		return 1;
	}

	while ((ch = fgetc(fp)) != EOF) {
		write(sockfd, &ch, 1);
	}

	pclose(fp);
	return 0;
}
