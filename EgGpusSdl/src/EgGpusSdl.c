#include "EgGpusSdl.h"
#include <EgGpus.h>
#include <EgShapes.h>
#include <SDL3/SDL_gpu.h>

#include "EgGpusDevice.h"
#include "EgGpusTexture.h"

void EgGpusSdlImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgGpusSdl);
	ecs_set_name_prefix(world, "EgGpusSdl");
	ecs_entity_t module = ecs_get_scope(world);
	ecs_set_scope(world, 0);
	ECS_IMPORT(world, EgGpus);
	ECS_IMPORT(world, EgShapes);
	ecs_set_scope(world, module);

	ecs_set_hooks(world, EgGpusDevice,
	{
	.on_add    = EgGpusDevice_add,
	.on_remove = EgGpusDevice_remove,
	});
	ecs_set_hooks(world, EgGpusTexture,
	{
	.on_add    = EgGpusTexture_add,
	.on_remove = EgGpusTexture_remove,
	});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity   = ecs_entity(world, {.name = "EgGpusDevice_Create"}),
	.callback = EgGpusDevice_Create,
	.phase    = EcsOnUpdate,
	.query.terms =
	{
	{.id = ecs_id(EgGpusDeviceCreateInfo), .src.id = EcsSelf},
	{.id = ecs_id(EgGpusDevice), .oper = EcsNot}, // Adds this
	}});

	// Recreate textures when the associated rectangle changes
	ecs_observer_init(world,
	&(ecs_observer_desc_t){
	.entity      = ecs_entity(world, {.name = "Observer_EgGpuTexture"}),
	.callback    = EgGpusTexture_Observer,
	.events      = {EcsOnSet},
	.query.terms = {
	{.id = ecs_id(EgShapesRectangle)},
	{.id = ecs_id(EgGpusTexture), .inout = EcsInOutFilter},
	{.id = ecs_id(EgGpusDevice), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsInOutFilter},
	{.id = ecs_id(EgGpusTextureCreateInfo), .oper = EcsOptional, .inout = EcsInOutFilter},
	}});
}
