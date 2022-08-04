#include "eg_util.h"


char const ** get_entity_names_from_filter(ecs_world_t * world, ecs_filter_t *f)
{
	ecs_iter_t it;
	it = ecs_filter_iter(world, f);
	int32_t n = ecs_iter_count(&it);
	it = ecs_filter_iter(world, f);
	char const ** r = ecs_os_malloc_n(char const*, n+1);
	int32_t j = 0;
	while (ecs_filter_next(&it))
	{
		for (int32_t i = 0; i < it.count; i ++)
		{
			char const * name = ecs_get_name(world, it.entities[i]);
			r[j] = name;
			j++;
		}
	}
	r[j] = NULL;
	return r;
}



void const * eg_get_first_from_filter(ecs_world_t * world, ecs_filter_t *f)
{
	ecs_iter_t it = ecs_filter_iter(world, f);
	while (ecs_filter_next(&it))
	{
		if (it.count <= 0) {return NULL;}
		if (it.ptrs == NULL) {return NULL;}
		return it.ptrs[0];
	}
	return NULL;
}


void eg_add_all_from_filter(ecs_world_t * world, ecs_entity_t e, ecs_filter_t *f)
{
	ecs_iter_t it = ecs_filter_iter(world, f);
	while (ecs_filter_next(&it))
	{
		if (it.count <= 0) {return;}
		if (it.ptrs == NULL) {return;}
		for (int32_t i = 0; i < it.count; i ++)
		{
			ecs_entity_t a = it.entities[i];
			ecs_add_id(world, e, a);
		}
	}
	return;
}
