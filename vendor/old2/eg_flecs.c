#include "eg_flecs.h"
#include <stdio.h>

void eg_iter_info(ecs_iter_t *it, const char *fname, const char *file, int32_t line)
{
	ecs_world_t * world = it->world;
	for (int i = 0; i < it->count; i ++)
	{
		ecs_entity_t e = it->entities[i];
		if (ecs_is_valid(world, e))
		{
			char const * name = ecs_get_name(world, e);
			ecs_type_t const * type = ecs_get_type(world, e);
			char const * typestr = ecs_type_str(world, type);
			fprintf(stdout,
			ECS_YELLOW "EG %s:%i:" ECS_CYAN "%s()"ECS_NORMAL": 0x%08llx: name=%s, type=(%s)\n",
			file, line, fname, e, name, typestr
			);
		}
		else
		{
			fprintf(stdout,
			ECS_YELLOW "EG %s:%i:" ECS_CYAN "%s()"ECS_NORMAL": 0x%08llx: invalid\n",
			file, line, fname, e);
		}
	}
}

