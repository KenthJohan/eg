#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include "flecs.h"

#define EG_U64BITSET_ON(x,c) ((x)[(uint64_t)(c) >> 6] |= (UINT64_C(1) << ((c) & 63)))
#define EG_U64BITSET_OFF(x,c) ((x)[(uint64_t)(c) >> 6] &= ~(UINT64_C(1) << ((c) & 63)))
#define EG_U64BITSET_GET(x,c) (!!(((x)[(uint64_t)(c) >> 6] >> ((c)&63)) & 1))
#define EG_CLAMP(x, x0, x1) (((x) > (x1)) ? (x1) : (((x) < (x0)) ? (x0) : (x)))

#if 1
#define EG_ASSERT(x) (void)((!!(x)) || (eg_soft_assert(__FILE__, __LINE__, #x),0) || (__builtin_trap(),0))
static void eg_soft_assert(char const * file, int line, char const * e)
{
	fprintf(stderr, "%s:%i: %s\n", file, line, e);
}
#else
#define EG_ASSERT(x)
#endif





#if 1
#define EG_TRACE(...) eg_trace(__FILE__, __LINE__, __VA_ARGS__)
static void eg_trace(const char *file, int32_t line, const char *fmt, ...)
{
	char buf[1024];
	char * p = buf;
	va_list args;
	va_start(args, fmt);
	p += snprintf(p, 512, ECS_YELLOW"EG %s:%i "ECS_NORMAL, file, line);
	vsnprintf(p, 512, fmt, args);
	fputs(buf, stdout);
	fputc('\n', stdout);
	va_end(args);
}
#else
#define EG_TRACE(...)
#endif



#define EG_ITER_INFO(it) eg_iter_info((it), __FUNCTION__, __FILE__, __LINE__);
static void eg_iter_info(ecs_iter_t *it, const char *f, const char *file, int32_t line)
{
	ecs_world_t * world = it->world;
	for (int i = 0; i < it->count; i ++)
	{
		ecs_entity_t e = it->entities[i];
		fprintf(stdout,
		ECS_YELLOW"EG %s() %s:%i "ECS_NORMAL"0x%08llx: name=%s, type=(%s)\n",
		f, file, line, e, ecs_get_name(world, e), ecs_type_str(world, ecs_get_type(world, e))
		);
	}
}
