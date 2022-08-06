#include "render1.h"

#include "EgVkInstances.h"
#include "EgWindows.h"

ecs_entity_t render1_init(ecs_world_t * world)
{
	ecs_entity_t entity_instance = ecs_new(world, 0);
	ecs_add_pair(world, entity_instance, EcsChildOf, ecs_id(EgVkInstances));
	ecs_add(world, entity_instance, EgWindow);
	return entity_instance;
}
