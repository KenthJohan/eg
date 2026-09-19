#include "EgGpus.h"

ECS_COMPONENT_DECLARE(EgGpusDevice);
ECS_COMPONENT_DECLARE(EgGpusDeviceCreateInfo);
ECS_COMPONENT_DECLARE(EgGpusTexture);
ECS_COMPONENT_DECLARE(EgGpusTextureCreateInfo);
ECS_COMPONENT_DECLARE(EgGpusLocation);
ECS_COMPONENT_DECLARE(EgGpusGraphicsPipeline);
ECS_COMPONENT_DECLARE(EgGpusGraphicsPipelineCreateInfo);
ECS_COMPONENT_DECLARE(EgGpusShaderVertex);
ECS_COMPONENT_DECLARE(EgGpusShaderVertexCreateInfo);
ECS_COMPONENT_DECLARE(EgGpusShaderFragment);
ECS_COMPONENT_DECLARE(EgGpusShaderFragmentCreateInfo);

void EgGpusImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgGpus);
	ecs_set_name_prefix(world, "EgGpus");

	ECS_COMPONENT_DEFINE(world, EgGpusDevice);
	ECS_COMPONENT_DEFINE(world, EgGpusDeviceCreateInfo);
	ECS_COMPONENT_DEFINE(world, EgGpusTexture);
	ECS_COMPONENT_DEFINE(world, EgGpusTextureCreateInfo);
	ECS_COMPONENT_DEFINE(world, EgGpusLocation);
	ECS_COMPONENT_DEFINE(world, EgGpusGraphicsPipeline);
	ECS_COMPONENT_DEFINE(world, EgGpusGraphicsPipelineCreateInfo);
	ECS_COMPONENT_DEFINE(world, EgGpusShaderVertex);
	ECS_COMPONENT_DEFINE(world, EgGpusShaderVertexCreateInfo);
	ECS_COMPONENT_DEFINE(world, EgGpusShaderFragment);
	ECS_COMPONENT_DEFINE(world, EgGpusShaderFragmentCreateInfo);

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

	ecs_struct(world,
	{.entity = ecs_id(EgGpusTextureCreateInfo),
	.members = {
	{.name = "sample_count", .type = ecs_id(ecs_u32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgGpusLocation),
	.members = {
	{.name = "location", .type = ecs_id(ecs_i32_t)},
	}});
	
	ecs_struct(world,
	{.entity = ecs_id(EgGpusGraphicsPipeline),
	.members = {
	{.name = "object", .type = ecs_id(ecs_uptr_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgGpusGraphicsPipelineCreateInfo),
	.members = {
	{.name = "sample_count", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgGpusShaderVertex),
	.members = {
	{.name = "object", .type = ecs_id(ecs_uptr_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgGpusShaderVertexCreateInfo),
	.members = {
	{.name = "path", .type = ecs_id(ecs_string_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgGpusShaderFragment),
	.members = {
	{.name = "object", .type = ecs_id(ecs_uptr_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgGpusShaderFragmentCreateInfo),
	.members = {
	{.name = "path", .type = ecs_id(ecs_string_t)},
	}});
}
