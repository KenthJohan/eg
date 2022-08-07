#include "EgLogs.h"
#include <stdio.h>

ECS_DECLARE(EgLogsError);
ECS_DECLARE(EgLogsVerbose);
ECS_DECLARE(EgLogsInfo);

ECS_DECLARE(EgEventsIdling);
ECS_DECLARE(EgEventsPrint);


void eg_event_submit(ecs_world_t * world, ecs_entity_t channel, void * data, size_t size)
{
	if (ecs_has(world, channel, EgEventsPrint))
	{
		char const * cstring = data;
		fputs(cstring, stdout);
	}
	if (ecs_has(world, channel, EgEventsIdling))
	{
		while(1)
		{
			ecs_progress(world, 0);
			ecs_os_sleep(0, 1000000);
		}
	}
}


void eg_event_strf(ecs_world_t * world, ecs_entity_t channel, const char *file, int32_t line, const char *fmt, ...)
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
	eg_event_submit(world, channel, buf, 0);
	va_end(args);
}




void EgLogsImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgLogs);
	ecs_set_name_prefix(world, "Eg");

	ECS_TAG_DEFINE(world, EgLogsError);
	ECS_TAG_DEFINE(world, EgLogsVerbose);
	ECS_TAG_DEFINE(world, EgLogsInfo);

	ECS_TAG_DEFINE(world, EgEventsIdling);
	ECS_TAG_DEFINE(world, EgEventsPrint);

	ecs_add(world, EgLogsError, EgEventsIdling);

	ecs_add(world, EgLogsError,    EgEventsPrint);
	ecs_add(world, EgLogsVerbose,  EgEventsPrint);
	ecs_add(world, EgLogsInfo,     EgEventsPrint);

}




