#pragma once
#include <stdint.h>

/**
 * @brief Convert fs_readfile error to string
 *
 * @param filename filename
 * @param buf out error buffer string
 * @param len error buffer string length
 * @return int the length that got written to buffer string
 */
int eg_fs_readfile_failed_reason(char const *filename, char *buf, int len);

/** Prints current working directory */
void eg_fs_pwd();

/** Read whole file and allocates memory
 *
 * @param path filename to read
 */
char *eg_fs_readfile(char const *path);


char *eg_fs_readfile_and_size(const char *path, int32_t *out_size);


void eg_str_replace_ab(char *str, char a, char b);