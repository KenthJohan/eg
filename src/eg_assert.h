#pragma once

void eg_assert_soft(char const * file, int line, char const * e);

#if 1
#define EG_ASSERT(x) (void)((!!(x)) || (eg_assert_soft(__FILE__, __LINE__, #x),0) || (__builtin_trap(),0))
#else
#define EG_ASSERT(x)
#endif