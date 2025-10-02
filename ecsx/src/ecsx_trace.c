#include "ecsx/ecsx_trace.h"
#include <stdio.h>
#include <printf.h>

void ecsx_trace_system_iter(ecs_iter_t *it)
{
	const ecs_type_t *type = NULL;
	char *str_type = NULL;
	char *str_path = NULL;
	char *str_query = NULL;
	if (it->system != 0) {
		str_path = ecs_get_path(it->world, it->system);
		type = ecs_get_type(it->world, it->system);
		if (type) {
			str_type = ecs_type_str(it->world, type);
		}
	}
	if (it->query) {
		str_query = ecs_query_str(it->query);
	}
	ecs_trace("System '%s', count=%i", str_path, it->count);
	ecs_log_push_(0);
	ecs_trace("Query '%s'", str_query);
	ecs_trace("Type '%s'", str_type);
	ecs_log_pop_(0);
	ecs_os_free(str_query);
	ecs_os_free(str_path);
	ecs_os_free(str_type);
}

void ecsx_trace_path(ecs_world_t *world, ecs_entity_t e, char const *prefix)
{
	char *str_path = ecs_get_path(world, e);
	ecs_trace("%s'%s'", prefix, str_path);
	ecs_os_free(str_path);
}

void ecsx_trace_type(ecs_world_t *world, ecs_entity_t e, char const *prefix)
{
	const ecs_type_t *type = ecs_get_type(world, e);
	char *str_type = ecs_type_str(world, type);
	ecs_trace("%s'%s'", prefix, str_type);
	ecs_os_free(str_type);
}

void ecsx_trace_ent(ecs_world_t *world, ecs_entity_t e, char const *prefix)
{
	char *str_path = ecs_get_path(world, e);
	const ecs_type_t *type = ecs_get_type(world, e);
	char *str_type = ecs_type_str(world, type);
	ecs_trace("%s%s '%s'", prefix, str_path, str_type);
	ecs_os_free(str_type);
	ecs_os_free(str_path);
}