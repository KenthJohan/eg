#pragma once
#include <stdarg.h>


#define EG_U64BITSET_ON(x,c) ((x)[(uint64_t)(c) >> 6] |= (UINT64_C(1) << ((c) & 63)))
#define EG_U64BITSET_OFF(x,c) ((x)[(uint64_t)(c) >> 6] &= ~(UINT64_C(1) << ((c) & 63)))
#define EG_U64BITSET_GET(x,c) (!!(((x)[(uint64_t)(c) >> 6] >> ((c)&63)) & 1))
#define EG_CLAMP(x, x0, x1) (((x) > (x1)) ? (x1) : (((x) < (x0)) ? (x0) : (x)))
#define EG_MAX(a,b) ((a)>(b)?(a):(b))
#define EG_MIN(a,b) ((a)<(b)?(a):(b))



void eg_assert_soft(char const * file, int line, char const * e);

#if 1
#define EG_ASSERT(x) (void)((!!(x)) || (eg_assert_soft(__FILE__, __LINE__, #x),0) || (__builtin_trap(),0))
#else
#define EG_ASSERT(x)
#endif

