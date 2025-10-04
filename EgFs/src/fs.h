#pragma once
#include <stddef.h>
#include <stdint.h>

#define FS_PATH_NONE   0x00
#define FS_PATH_FOLDER 0x01
#define FS_PATH_FILE   0x02
#define FS_PATH_LINK   0x04
#define FS_PATH_OTHER  0x08

char *fs_load_from_file(const char *filename, size_t *size);

uint32_t fs_get_path_flags(const char *path);

