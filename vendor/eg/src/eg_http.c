#include "eg/eg_http.h"
#include <stddef.h>

// https://github.com/JeffreytheCoder/Simple-HTTP-Server/blob/master/server.c

// https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types/Common_types
const char *eg_http_get_mime_type(const char *file_ext)
{
	if (strcasecmp(file_ext, "html") == 0 || strcasecmp(file_ext, "htm") == 0) {
		return "text/html";
	} else if (strcasecmp(file_ext, "txt") == 0) {
		return "text/plain";
	} else if (strcasecmp(file_ext, "jpg") == 0 || strcasecmp(file_ext, "jpeg") == 0) {
		return "image/jpeg";
	} else if (strcasecmp(file_ext, "js") == 0) {
		return "text/javascript";
	} else if (strcasecmp(file_ext, "png") == 0) {
		return "image/png";
	} else if (strcasecmp(file_ext, "ico") == 0) {
		return "image/vnd.microsoft.icon";
	} else if (strcasecmp(file_ext, "css") == 0) {
		return "text/css";
	}
	return NULL;
}
