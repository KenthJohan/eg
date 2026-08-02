#pragma once
#include <stddef.h>
#include <stdint.h>

char *eg_file_load_alloc(const char *filename, size_t *length);

#define FS_PATH_NONE 0x00
#define FS_PATH_DIR 0x01
#define FS_PATH_FILE 0x02
#define FS_PATH_LINK 0x04
#define FS_PATH_OTHER 0x08
uint32_t eg_file_get_path_flags(const char *path);