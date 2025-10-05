#include "fs.h"
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <flecs.h>


uint32_t fs_get_path_flags(const char *path)
{
	struct stat path_stat;
	if (stat(path, &path_stat) != 0) {
		return FS_PATH_NONE; // Error accessing the path
	}

	if (S_ISREG(path_stat.st_mode)) {
		return FS_PATH_FILE;
	} else if (S_ISDIR(path_stat.st_mode)) {
		return FS_PATH_DIR;
	} else {
		return FS_PATH_OTHER;
	}
}

char *fs_load_from_file(const char *filename, size_t *size)
{
	FILE *file;
	char *content = NULL;
	int32_t bytes;

	/* Open file for reading */
	ecs_os_fopen(&file, filename, "r");
	if (!file) {
		ecs_err("%s (%s)", ecs_os_strerror(errno), filename);
		goto error;
	}

	/* Determine file size */
	fseek(file, 0, SEEK_END);
	bytes = (int32_t)ftell(file);
	if (bytes == -1) {
		goto error;
	}
	fseek(file, 0, SEEK_SET);

	/* Load contents in memory */
	content = ecs_os_malloc(bytes + 1);
	*size = (size_t)bytes;
	if (!(*size = fread(content, 1, *size, file)) && bytes) {
		ecs_err("%s: read zero bytes instead of %d", filename, *size);
		ecs_os_free(content);
		content = NULL;
		goto error;
	} else {
		content[*size] = '\0';
	}

	fclose(file);

	return content;
error:
	if (file) {
		fclose(file);
	}
	ecs_os_free(content);
	return NULL;
}