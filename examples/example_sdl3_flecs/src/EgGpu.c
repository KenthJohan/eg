#include "EgGpu.h"
#include <egbase.h>

#include "EgGpu_System_EgGpuShader.h"
#include "EgGpu_System_EgGpuPipeline.h"
#include "EgGpu_System_EgGpuDevice.h"
#include "EgGpu_System_EgGpuBuffer.h"

ECS_COMPONENT_DECLARE(EgGpuDevice);
ECS_COMPONENT_DECLARE(EgGpuDeviceCreateInfo);
ECS_COMPONENT_DECLARE(EgGpuShaderVertex);
ECS_COMPONENT_DECLARE(EgGpuShaderVertexCreateInfo);
ECS_COMPONENT_DECLARE(EgGpuShaderFragment);
ECS_COMPONENT_DECLARE(EgGpuShaderFragmentCreateInfo);
ECS_COMPONENT_DECLARE(EgGpuPipeline);
ECS_COMPONENT_DECLARE(EgGpuPipelineCreateInfo);
ECS_COMPONENT_DECLARE(EgGpuDrawCube);
ECS_COMPONENT_DECLARE(EgGpuBuffer);
ECS_COMPONENT_DECLARE(EgGpuBufferCreateInfo);

void EgGpuImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgGpu);
	ECS_IMPORT(world, EgBase);
	ecs_set_name_prefix(world, "EgGpu");

	ECS_COMPONENT_DEFINE(world, EgGpuDevice);
	ECS_COMPONENT_DEFINE(world, EgGpuDeviceCreateInfo);
	ECS_COMPONENT_DEFINE(world, EgGpuShaderVertex);
	ECS_COMPONENT_DEFINE(world, EgGpuShaderVertexCreateInfo);
	ECS_COMPONENT_DEFINE(world, EgGpuShaderFragment);
	ECS_COMPONENT_DEFINE(world, EgGpuShaderFragmentCreateInfo);
	ECS_COMPONENT_DEFINE(world, EgGpuPipeline);
	ECS_COMPONENT_DEFINE(world, EgGpuPipelineCreateInfo);
	ECS_COMPONENT_DEFINE(world, EgGpuDrawCube);
	ECS_COMPONENT_DEFINE(world, EgGpuBuffer);
	ECS_COMPONENT_DEFINE(world, EgGpuBufferCreateInfo);

	ecs_struct(world,
	{.entity = ecs_id(EgGpuDeviceCreateInfo),
	.members = {
	{.name = "debug", .type = ecs_id(ecs_bool_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgGpuDevice),
	.members = {
	{.name = "device", .type = ecs_id(ecs_uptr_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgGpuShaderVertexCreateInfo),
	.members = {
	{.name = "stage", .type = ecs_id(ecs_u32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgGpuShaderVertex),
	.members = {
	{.name = "device", .type = ecs_id(ecs_uptr_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgGpuShaderFragmentCreateInfo),
	.members = {
	{.name = "stage", .type = ecs_id(ecs_u32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgGpuShaderFragment),
	.members = {
	{.name = "device", .type = ecs_id(ecs_uptr_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgGpuPipelineCreateInfo),
	.members = {
	{.name = "sample_count", .type = ecs_id(ecs_u32_t)},
	{.name = "target_info_has_depth_stencil_target", .type = ecs_id(ecs_bool_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgGpuPipeline),
	.members = {
	{.name = "object", .type = ecs_id(ecs_uptr_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgGpuBuffer),
	.members = {
	{.name = "object", .type = ecs_id(ecs_uptr_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgGpuBufferCreateInfo),
	.members = {
	{.name = "usage", .type = ecs_id(ecs_u32_t)},
	}});

	ecs_system(world,
	{.entity = ecs_entity(world, {.name = "System_EgGpuDevice_Create", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_EgGpuDevice_Create,
	.query.terms =
	{
	{.id = ecs_id(EgGpuDeviceCreateInfo), .src.id = EcsSelf},
	{.id = ecs_id(EgGpuDevice), .oper = EcsNot}, // Adds this
	{.id = EgBaseUpdate},
	{.id = EgBaseError, .oper = EcsNot}
	}});

	ecs_system(world,
	{.entity = ecs_entity(world, {.name = "System_EgGpuShaderFragment_Create", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_EgGpuShaderFragment_Create,
	.query.terms =
	{
	{.id = ecs_id(EgGpuDevice), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsIn},
	{.id = ecs_id(EgGpuShaderFragmentCreateInfo), .src.id = EcsSelf},
	{.id = ecs_id(EgGpuShaderFragment), .oper = EcsNot}, // Adds this
	{.id = EgBaseUpdate},
	{.id = EgBaseError, .oper = EcsNot}
	}});

	ecs_system(world,
	{.entity = ecs_entity(world, {.name = "System_EgGpuShaderVertex_Create", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_EgGpuShaderVertex_Create,
	.query.terms =
	{
	{.id = ecs_id(EgGpuDevice), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsIn},
	{.id = ecs_id(EgGpuShaderVertexCreateInfo), .src.id = EcsSelf},
	{.id = ecs_id(EgGpuShaderVertex), .oper = EcsNot}, // Adds this
	{.id = EgBaseUpdate},
	{.id = EgBaseError, .oper = EcsNot}
	}});

	ecs_system(world,
	{.entity = ecs_entity(world, {.name = "System_EgGpuPipeline_Create", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_EgGpuPipeline_Create,
	.query.terms = {
	{.id = ecs_id(EgGpuDevice), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsIn},
	{.id = ecs_id(EgGpuPipelineCreateInfo), .src.id = EcsSelf},
	{.id = ecs_id(EgGpuShaderVertex), .trav = EcsDependsOn, .src.id = EcsUp},
	{.id = ecs_id(EgGpuShaderFragment), .trav = EcsDependsOn, .src.id = EcsUp},
	{.id = ecs_id(EgGpuPipeline), .oper = EcsNot}, // Adds this
	{.id = EgBaseUpdate},
	{.id = EgBaseError, .oper = EcsNot}
	}});

	ecs_system(world,
	{.entity = ecs_entity(world, {.name = "System_EgGpuBuffer_Create", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_EgGpuBuffer_Create,
	.query.terms = {
	{.id = ecs_id(EgGpuDevice), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsIn},
	{.id = ecs_id(EgGpuBufferCreateInfo), .src.id = EcsSelf},
	{.id = ecs_id(EgGpuBuffer), .oper = EcsNot}, // Adds this
	{.id = EgBaseUpdate},
	{.id = EgBaseError, .oper = EcsNot}
	}});
}
