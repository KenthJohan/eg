#pragma once

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
