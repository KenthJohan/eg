#include "ecsx/ecsx_file.h"

#include <errno.h>
#include <stdio.h>

#include <flecs.h>

char *ecsx_file_load_alloc(const char *filename, size_t *length)
{
	char   *content = NULL;
	int32_t bytes;
	size_t  size;
	if (length) {
		*length = 0;
	}

	FILE *file = ecs_os_fopen(filename, "r");
	if (!file) {
		ecs_err("%s (%s)", ecs_os_strerror(errno), filename);
		goto error;
	}

	fseek(file, 0, SEEK_END);
	bytes = (int32_t)ftell(file);
	if (bytes == -1) {
		goto error;
	}
	if (length) {
		*length = bytes;
	}
	fseek(file, 0, SEEK_SET);

	content = ecs_os_malloc(bytes + 1);
	size    = (size_t)bytes;
	if (!(size = fread(content, 1, size, file)) && bytes) {
		ecs_err("%s: read zero bytes instead of %d", filename, size);
		ecs_os_free(content);
		content = NULL;
		goto error;
	} else {
		content[size] = '\0';
	}

	fclose(file);

	return content;
error:
	ecs_os_free(content);
	return NULL;
}
