#pragma once
#include <stdarg.h>
#include "flecs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EG_U64BITSET_ON(x,c) ((x)[(uint64_t)(c) >> 6] |= (UINT64_C(1) << ((c) & 63)))
#define EG_U64BITSET_OFF(x,c) ((x)[(uint64_t)(c) >> 6] &= ~(UINT64_C(1) << ((c) & 63)))
#define EG_U64BITSET_GET(x,c) (!!(((x)[(uint64_t)(c) >> 6] >> ((c)&63)) & 1))
#define EG_CLAMP(x, x0, x1) (((x) > (x1)) ? (x1) : (((x) < (x0)) ? (x0) : (x)))
#define EG_MAX(a,b) ((a)>(b)?(a):(b))
#define EG_MIN(a,b) ((a)<(b)?(a):(b))

void eg_soft_assert(char const * file, int line, char const * e);
void eg_iter_info(ecs_iter_t *it, const char *fname, const char *file, int32_t line);

#if 1
#define EG_ASSERT(x) (void)((!!(x)) || (eg_soft_assert(__FILE__, __LINE__, #x),0) || (__builtin_trap(),0))
#else
#define EG_ASSERT(x)
#endif




#define EG_ITER_INFO(it) eg_iter_info((it), __FUNCTION__, __FILE__, __LINE__);



void eg_basics_enable_ternminal_color();


#ifdef __cplusplus
}
#endif
