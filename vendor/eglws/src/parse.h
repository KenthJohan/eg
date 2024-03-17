#pragma once

#include <stdint.h>

const char* parse_c_digit(const char *ptr,int64_t *value_out);

char * parse_string(char const *p, char const * needle);

void parse_command(char const * in, int len);

void parse_test();