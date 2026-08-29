#include "EgGpus.h"

ECS_COMPONENT_DECLARE(EgGpusDevice);
ECS_COMPONENT_DECLARE(EgGpusDeviceCreateInfo);
ECS_COMPONENT_DECLARE(EgGpusTexture);
ECS_COMPONENT_DECLARE(EgGpusTextureCreateInfo);

void EgGpusImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgGpus);
	ecs_set_name_prefix(world, "EgGpus");

	ECS_COMPONENT_DEFINE(world, EgGpusDevice);
	ECS_COMPONENT_DEFINE(world, EgGpusDeviceCreateInfo);
	ECS_COMPONENT_DEFINE(world, EgGpusTexture);
	ECS_COMPONENT_DEFINE(world, EgGpusTextureCreateInfo);

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

	ecs_struct(world,
	{.entity = ecs_id(EgGpusTexture),
	.members = {
	{.name = "object", .type = ecs_id(ecs_uptr_t)},
	}});
}
