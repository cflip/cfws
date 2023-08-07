#include "http.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

ssize_t http_parse_request_line(const char *reqline, struct http_request *req_out)
{
	char uribuf[CFWS_MAXURI];
	size_t urilen = 0, query_len = 0;

	const char *ch = reqline;

	/* Quick 'n' dirty strncmp to determine the request method. */
	if (strncmp(reqline, "GET ", 4) == 0) {
		req_out->method = HTTP_METHOD_GET;
		ch += 4;
	} else if (strncmp(reqline, "POST ", 5) == 0) {
		req_out->method = HTTP_METHOD_POST;
		ch += 5;
	} else {
		fprintf(stderr, "Unhandled request type: '%s'\n", reqline);
		return -1;
	}

	/* The first char after the first space should be a slash, otherwise
	 * there's either an invalid URI or something we don't handle yet. */
	if (*ch != '/') {
		fprintf(stderr, "Invalid URI string: '%s'\n", ch);
		return -1;
	}

	/* Copy URI path into a buffer, until we reach a space, query string, or
	 * one of the buffers reaches the end. */
	while (*ch != ' ' && *ch != '?' && *ch != 0 && urilen < CFWS_MAXURI) {
		uribuf[urilen++] = *ch;
		++ch;
	}

	req_out->uri = malloc(urilen + 1);
	memcpy(req_out->uri, uribuf, urilen);
	req_out->uri[urilen] = '\0';

	/* Parse the query string if one exists. */
	if (*ch == '?') {
		ch++; /* Skip the question mark at the beginning. */

		while (*ch != ' ' && *ch != 0 && query_len < CFWS_MAXURI) {
			uribuf[query_len++] = *ch;
			++ch;
		}

		req_out->query_str = malloc(query_len + 1);
		memcpy(req_out->query_str, uribuf, query_len);
		req_out->query_str[query_len] = '\0';
	}

	/* Lastly, read the HTTP version and for now make sure it's HTTP/1.1. */
	if (strncmp(ch, " HTTP/1.1\r\n", 9) != 0) {
		fprintf(stderr, "Invalid HTTP version: '%s'", ch);
		return -1;
	}
	
	ch += 9;
	return ch - reqline;
}

struct http_request http_parse_request(const char *reqstr)
{
	const char *current_line = reqstr;
	bool has_parsed_header = false;

	struct http_request req;
	req.method    = HTTP_METHOD_UNKNOWN;
	req.uri       = NULL;
	req.query_str = NULL;
	req.cookie    = NULL;
	req.body      = NULL;

	while (current_line) {
		char *next_line = strstr(current_line, "\r\n");
		size_t line_length = 0;
		if (next_line) {
			line_length = next_line - current_line;
			*next_line = '\0';
			next_line++;
		}

		if (!has_parsed_header) {
			http_parse_request_line(current_line, &req);
			has_parsed_header = true;
		} else {
			if (line_length == 0) {
				/* An empty line means we are at the end of the
				 * request headers. */
				current_line = next_line ? (next_line + 1) : NULL;
				break;
			}

			if (strncmp(current_line, "Cookie: ", 8) == 0) {
				const char *cookie_str = current_line + 8;
				size_t cookie_length = next_line - cookie_str;

				req.cookie = malloc(cookie_length + 1);
				memcpy(req.cookie, cookie_str, cookie_length);
				req.cookie[cookie_length] = '\0';
			}
		}

		if (next_line)
			*next_line = '\n';
		current_line = next_line ? (next_line + 1) : NULL;
	}

	/* Copy the request body if we have one */
	if (current_line && *current_line != '\0') {
		size_t body_len;
		body_len = strlen(current_line);
		req.body = malloc(body_len + 1);
		memcpy(req.body, current_line, body_len);
		req.body[body_len] = '\0';
	}

	return req;
}

void http_free_request(struct http_request *req)
{
	free(req->uri);
	if (req->query_str)
		free(req->query_str);
	if (req->cookie)
		free(req->cookie);
	if (req->body)
		free(req->body);
}

static const char *response_msg[] = {
	"200 OK",
	"400 Bad Request",
	"404 Not Found",
	"501 Not Implemented"
};

void http_response_statusline(enum http_res_code status_code, int sockfd)
{
	char statusline[64];
	int len;
	len = snprintf(statusline, 64, "HTTP/1.1 %s\r\n", response_msg[status_code]);
	write(sockfd, statusline, len);
}
