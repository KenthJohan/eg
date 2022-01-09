#pragma once

#if 1
#define EG_ASSERT(x) (void)((!!(x)) || (eg_soft_assert(__FILE__, __LINE__, #x),0) || (__builtin_trap(),0))
void eg_soft_assert(char const * file, int line, char const * e)
{
	fprintf(stderr, "%s:%i: %s\n", file, line, e);
}
#else
#define EG_ASSERT(x)
#endif

#if 0
#define EG_TRACE(...) eg_trace(__FILE__, __LINE__, __VA_ARGS__)
void eg_trace(const char *file, int32_t line, const char *fmt, ...)
{
	char buf[1024];
	char * p = buf;
	va_list args;
	va_start(args, fmt);
	p += snprintf(p, 512, "%s:%i:", file, line);
	vsnprintf(p, 512, fmt, args);
	fputs(buf, stdout);
	va_end(args);
}
#else
#define EG_TRACE(...)
#endif