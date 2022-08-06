#include "EgLogs.h"
#include <stdio.h>

ECS_DECLARE(EgLogsError);
ECS_DECLARE(EgLogsVerbose);
ECS_DECLARE(EgLogsInfo);





void eg_trace(ecs_world_t * world, ecs_entity_t channel, const char *file, int32_t line, const char *fmt, ...)
{
	//char const * name = ecs_get_name(world, channel);
	char const * name = ecs_get_fullpath(world, channel);
	char buf[1024];
	char * p = buf;
	va_list args;
	va_start(args, fmt);
	//p += snprintf(p, 512, ECS_YELLOW "EG %s:%i " ECS_NORMAL, file, line);
	p += snprintf(p, 512, ECS_YELLOW "%s " ECS_NORMAL, name);
	p += vsnprintf(p, 512, fmt, args);
	fputs(buf, stdout);
	va_end(args);
}




void EgLogsImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgLogs);
	ecs_set_name_prefix(world, "Eg");

	ECS_TAG_DEFINE(world, EgLogsError);
	ECS_TAG_DEFINE(world, EgLogsVerbose);
	ECS_TAG_DEFINE(world, EgLogsInfo);

}




