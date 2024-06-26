#include "egmisc/eg_log.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

void eg_log_(char const *format, ...)
{
	assert(format);
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}