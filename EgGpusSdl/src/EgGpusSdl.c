#include "EgGpusSdl.h"
#include <EgGpus.h>
#include <EgFs.h>
#include <EgShapes.h>
#include <SDL3/SDL_gpu.h>

#include "EgGpusDevice.h"
#include "EgGpusBuffer.h"
#include "EgGpusSampler.h"
#include "EgGpusTexture.h"
#include "EgGpusGraphicsPipeline.h"
#include "EgGpusShader.h"

void EgGpusSdlImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgGpusSdl);
	ecs_set_name_prefix(world, "EgGpusSdl");
	ecs_entity_t module = ecs_get_scope(world);
	ecs_set_scope(world, 0);
	ECS_IMPORT(world, EgGpus);
	ECS_IMPORT(world, EgFs);
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
	ecs_set_hooks(world, EgGpusBuffer,
	{
	.on_add    = EgGpusBuffer_add,
	.on_remove = EgGpusBuffer_remove,
	});
	ecs_set_hooks(world, EgGpusSampler,
	{
	.on_add    = EgGpusSampler_add,
	.on_remove = EgGpusSampler_remove,
	});
	ecs_set_hooks(world, EgGpusGraphicsPipeline,
	{
	.on_remove = EgGpusGraphicsPipeline_remove,
	});
	ecs_set_hooks(world, EgGpusShaderVertex,
	{
	.on_remove = EgGpusShaderVertex_remove,
	});
	ecs_set_hooks(world, EgGpusShaderFragment,
	{
	.on_remove = EgGpusShaderFragment_remove,
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

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity   = ecs_entity(world, {.name = "EgGpusBuffer_Create"}),
	.callback = EgGpusBuffer_Create,
	.phase    = EcsOnUpdate,
	.query.terms =
	{
	{.id = ecs_id(EgGpusDevice), .trav = EcsChildOf, .src.id = EcsUp},
	{.id = ecs_id(EgGpusBufferCreateInfo), .src.id = EcsSelf},
	{.id = ecs_id(EgGpusBuffer), .oper = EcsNot}, // Adds this
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity   = ecs_entity(world, {.name = "EgGpusSampler_Create"}),
	.callback = EgGpusSampler_Create,
	.phase    = EcsOnUpdate,
	.query.terms =
	{
	{.id = ecs_id(EgGpusDevice), .trav = EcsChildOf, .src.id = EcsUp},
	{.id = ecs_id(EgGpusSamplerCreateInfo), .src.id = EcsSelf},
	{.id = ecs_id(EgGpusSampler), .oper = EcsNot}, // Adds this
	}});

	ecs_observer_init(world,
	&(ecs_observer_desc_t){
	.entity      = ecs_entity(world, {.name = "EgGpusTexture_Observer"}),
	.callback    = EgGpusTexture_Observer,
	.events      = {EcsOnSet},
	.query.terms = {
	{.id = ecs_id(EgShapesRectangle)},
	{.id = ecs_id(EgGpusTexture)},
	{.id = ecs_id(EgGpusDevice), .trav = EcsChildOf, .src.id = EcsUp},
	{.id = ecs_id(EgGpusTextureCreateInfo), .oper = EcsOptional},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgGpusGraphicsPipeline_Create"}),
	.callback    = EgGpusGraphicsPipeline_Create,
	.phase       = EcsOnUpdate,
	.query.terms = {
	{.id = ecs_id(EgGpusDevice), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsIn},
	{.id = ecs_id(EgGpusGraphicsPipelineCreateInfo), .src.id = EcsSelf},
	{.id = ecs_id(EcsComponent), .trav = EcsDependsOn, .src.id = EcsUp},
	{.id = ecs_id(EgGpusShaderVertex), .trav = EcsDependsOn, .src.id = EcsUp},
	{.id = ecs_id(EgGpusShaderFragment), .trav = EcsDependsOn, .src.id = EcsUp},
	{.id = ecs_id(EgGpusGraphicsPipeline), .oper = EcsNot}, // Adds this
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity   = ecs_entity(world, {.name = "EgGpusShader_Create"}),
	.callback = EgGpusShader_Create,
	.phase    = EcsOnUpdate,
	.query.terms =
	{
	{.id = ecs_id(EgGpusDevice), .src.id = EcsUp, .trav = EcsChildOf},
	{.id = ecs_id(EgGpusShaderCreateInfo), .src.id = EcsSelf},
	{.id = ecs_id(EgFsContent), .trav = EcsDependsOn, .src.id = EcsUp},
	{.id = ecs_id(EgGpusShaderVertex), .oper = EcsNot},   // Adds this
	{.id = ecs_id(EgGpusShaderFragment), .oper = EcsNot}, // Adds this
	}});
}
