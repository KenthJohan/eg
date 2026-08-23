#include "EgGpus.h"

ECS_COMPONENT_DECLARE(EgGpusDevice);
ECS_COMPONENT_DECLARE(EgGpusDeviceCreateInfo);

void EgGpusImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgGpus);
	ecs_set_name_prefix(world, "EgGpus");

	ECS_COMPONENT_DEFINE(world, EgGpusDevice);
	ECS_COMPONENT_DEFINE(world, EgGpusDeviceCreateInfo);

	ecs_struct(world,
	{.entity = ecs_id(EgGpusDeviceCreateInfo),
	.members = {
	{.name = "debug", .type = ecs_id(ecs_bool_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgGpusDevice),
	.members = {
	{.name = "device", .type = ecs_id(ecs_uptr_t)},
	}});
}
