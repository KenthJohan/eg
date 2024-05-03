#include "eg_fs.h"
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <flecs.h>

#define COLOR_FILENAME "\033[38;2;150;150;255m" // Blue
#define COLOR_CWD "\033[38;2;150;150;255m"      // Blue
#define COLOR_ERROR "\033[38;2;255;50;50m"      // Red
#define COLOR_RST "\033[0m"

int eg_fs_readfile_failed_reason(char const *filename, char *buf, int len)
{
	char const *e = strerror(errno);
	char cwd[1024] = {0};
	char *rc = getcwd(cwd, sizeof(cwd));
	rc = rc ? rc : "";
	int n = snprintf(buf, len, "cwd:%s%s%s path:%s%s%s error:%s%s%s",
	                 COLOR_CWD, rc, COLOR_RST,
	                 COLOR_FILENAME, filename, COLOR_RST,
	                 COLOR_ERROR, e, COLOR_RST);
	return n;
}

void eg_fs_pwd()
{
	char cwd[1024] = {0};
	char *rc = getcwd(cwd, sizeof(cwd));
	if (rc == NULL) {
		ecs_err("getcwd error: %s\n", strerror(errno));
		return;
	}
	printf("Current working directory: %s%s%s\n", COLOR_CWD, cwd, COLOR_RST);
}

char *eg_fs_readfile(char const *path)
{
	ecs_assert(path != NULL, ECS_INVALID_PARAMETER, "");
	char *content = NULL;

	FILE *file = fopen(path, "r");
	if (file == NULL) {
		goto error;
	}

	fseek(file, 0, SEEK_END);
	int32_t size = (int32_t)ftell(file);

	if (size == -1) {
		goto error;
	}
	rewind(file);

	content = malloc(size + 1);
	content[size] = '\0';
	size_t n = fread(content, size, 1, file);
	if (n != 1) {
		ecs_err("%s: could not read wholef file %d bytes\n", path, size);
		goto error;
	}
	ecs_assert(content[size] == '\0', ECS_INTERNAL_ERROR, "Expected null terminator");
	fclose(file);
	return content;
error:
	if (content) {
		free(content);
	}
	return NULL;
}



char *eg_fs_readfile_and_size(const char *path, int32_t *out_size)
{
	ecs_assert(path != NULL, ECS_INVALID_PARAMETER, "");
	char *content = NULL;

	FILE *file = fopen(path, "r");
	if (file == NULL) {
		goto error;
	}

	fseek(file, 0, SEEK_END);
	int32_t size = (int32_t)ftell(file);
	(*out_size) = size;

	if (size == -1) {
		goto error;
	}
	rewind(file);

	content = malloc(size + 1);
	content[size] = '\0';
	size_t n = fread(content, size, 1, file);
	if (n != 1) {
		ecs_err("%s: could not read wholef file %d bytes\n", path, size);
		goto error;
	}
	ecs_assert(content[size] == '\0', ECS_INTERNAL_ERROR, "Expected null terminator");
	fclose(file);
	return content;
error:
	if (content) {
		free(content);
	}
	return NULL;
}


void eg_str_replace_ab(char *str, char a, char b)
{
	char *p = str;
	while (p[0]) {
		if (p[0] == a) {
			p[0] = b;
		}
		p++;
	}
}