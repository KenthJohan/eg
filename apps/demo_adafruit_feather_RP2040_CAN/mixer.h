#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void mixer(int32_t f, int32_t r, int32_t res, int32_t t[4], int8_t m[4]);

#ifdef __cplusplus
}
#endif