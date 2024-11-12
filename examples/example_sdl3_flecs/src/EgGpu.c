#include "EgGpu.h"

#include <stdlib.h>
#include <stdio.h>

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_gpu.h>

#include "shader_spirv.h"
#include "vertex.h"

ECS_COMPONENT_DECLARE(EgGpuDevice);
ECS_COMPONENT_DECLARE(EgGpuDeviceCreateInfo);
ECS_COMPONENT_DECLARE(EgGpuShaderVertex);
ECS_COMPONENT_DECLARE(EgGpuShaderVertexCreateInfo);
ECS_COMPONENT_DECLARE(EgGpuShaderFragment);
ECS_COMPONENT_DECLARE(EgGpuShaderFragmentCreateInfo);
ECS_COMPONENT_DECLARE(EgGpuPipeline);
ECS_COMPONENT_DECLARE(EgGpuPipelineCreateInfo);

#define TESTGPU_SUPPORTED_FORMATS (SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXBC | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_METALLIB)

static void System_EgGpuDevice_Create(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;
	EgGpuDeviceCreateInfo *create = ecs_field(it, EgGpuDeviceCreateInfo, 0);
	char const *statustext = "";
	char const *color = "";

	ecs_log_set_level(1);
	ecs_dbg("System Sampler_Create() count:%i", it->count);
	ecs_log_push_1();
	for (int i = 0; i < it->count; ++i, ++create) {
		ecs_entity_t e = it->entities[i];
		ecs_dbg("Entity: '%s'", ecs_get_name(world, e));
		ecs_log_push_1();
		{
			SDL_GPUDevice *device = SDL_CreateGPUDevice(TESTGPU_SUPPORTED_FORMATS, true, NULL);
			if (device == NULL) {
				ecs_enable(world, e, false);
				continue;
			}
			ecs_set(world, e, EgGpuDevice, {.device = device});
			ecs_dbg("SDL_CreateGPUDevice() -> %p", device);
			char buf[128];
			snprintf(buf, 128, "%s [%s]", ecs_get_name(world, e), statustext);
			ecs_doc_set_name(world, e, buf);
			char const *driver = SDL_GetGPUDeviceDriver(device);
			SDL_GPUShaderFormat formats = SDL_GetGPUShaderFormats(device);
			snprintf(buf, 128, "Driver: %s, %08X", driver, formats);
			ecs_doc_set_brief(world, e, buf);
			ecs_doc_set_color(world, e, color);
		}
		ecs_log_pop_1();

	} // END FOR LOOP
	ecs_log_pop_1();
	ecs_log_set_level(0);
}

static void System_EgGpuShaderFragment_Create(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;
	EgGpuDevice *gpu = ecs_field(it, EgGpuDevice, 0);
	EgGpuShaderVertexCreateInfo *create = ecs_field(it, EgGpuShaderVertexCreateInfo, 1);
	ecs_log_set_level(1);
	ecs_dbg("System Sampler_Create() count:%i", it->count);
	ecs_log_push_1();
	for (int i = 0; i < it->count; ++i, ++create) {
		ecs_entity_t e = it->entities[i];
		ecs_dbg("Entity: '%s'", ecs_get_name(world, e));
		ecs_log_push_1();
		{
			//SDL_GPUShaderStage stage = create->stage;
			SDL_GPUShader *s = shader_spirv_compile(gpu->device, "shaders/cube", SDL_GPU_SHADERSTAGE_FRAGMENT);
			if (s == NULL) {
				ecs_enable(world, e, false);
				continue;
			}
			ecs_set(world, e, EgGpuShaderVertex, {.object = s});
			ecs_dbg("shader_spirv_compile() -> %p", s);
			//ecs_enable(world, e, false);
		}
		ecs_log_pop_1();

	} // END FOR LOOP
	ecs_log_pop_1();
	ecs_log_set_level(0);
}

static void System_EgGpuShaderVertex_Create(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;
	EgGpuDevice *gpu = ecs_field(it, EgGpuDevice, 0);
	EgGpuShaderFragmentCreateInfo *create = ecs_field(it, EgGpuShaderFragmentCreateInfo, 1);
	ecs_log_set_level(1);
	ecs_dbg("System Sampler_Create() count:%i", it->count);
	ecs_log_push_1();
	for (int i = 0; i < it->count; ++i, ++create) {
		ecs_entity_t e = it->entities[i];
		ecs_dbg("Entity: '%s'", ecs_get_name(world, e));
		ecs_log_push_1();
		{
			//SDL_GPUShaderStage stage = create->stage;
			SDL_GPUShader *s = shader_spirv_compile(gpu->device, "shaders/cube", SDL_GPU_SHADERSTAGE_VERTEX);
			if (s == NULL) {
				ecs_enable(world, e, false);
				continue;
			}
			ecs_set(world, e, EgGpuShaderFragment, {.object = s});
			ecs_dbg("shader_spirv_compile() -> %p", s);
			//ecs_enable(world, e, false);
		}
		ecs_log_pop_1();

	} // END FOR LOOP
	ecs_log_pop_1();
	ecs_log_set_level(0);
}

static void System_EgGpuPipeline_Create(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;
	EgGpuDevice *gpu = ecs_field(it, EgGpuDevice, 0);
	EgGpuPipelineCreateInfo *create = ecs_field(it, EgGpuPipelineCreateInfo, 1);
	EgGpuShaderVertex *sv = ecs_field(it, EgGpuShaderVertex, 2);
	EgGpuShaderFragment *sf = ecs_field(it, EgGpuShaderFragment, 3);
	ecs_log_set_level(1);
	ecs_dbg("System_EgGpuPipeline_Create() count:%i", it->count);
	ecs_log_push_1();
	for (int i = 0; i < it->count; ++i, ++create) {
		ecs_entity_t e = it->entities[i];
		ecs_dbg("Entity: '%s'", ecs_get_name(world, e));
		ecs_log_push_1();
		{
			SDL_GPUColorTargetDescription color_target_desc;
			SDL_GPUVertexAttribute vertex_attributes[2];
			SDL_GPUVertexBufferDescription vertex_buffer_desc;
			SDL_GPUGraphicsPipelineCreateInfo pipelinedesc = {0};
			//color_target_desc.format = SDL_GetGPUSwapchainTextureFormat(gpu->device, state->windows[0]);
			color_target_desc.format = SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM;
			pipelinedesc.target_info.num_color_targets = 1;
			pipelinedesc.target_info.color_target_descriptions = &color_target_desc;
			pipelinedesc.target_info.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D16_UNORM;
			pipelinedesc.target_info.has_depth_stencil_target = true;

			pipelinedesc.depth_stencil_state.enable_depth_test = true;
			pipelinedesc.depth_stencil_state.enable_depth_write = true;
			pipelinedesc.depth_stencil_state.compare_op = SDL_GPU_COMPAREOP_LESS_OR_EQUAL;

			pipelinedesc.multisample_state.sample_count = create[i].sample_count;

			pipelinedesc.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

			pipelinedesc.vertex_shader = sv[i].object;
			pipelinedesc.fragment_shader = sf[i].object;

			vertex_buffer_desc.slot = 0;
			vertex_buffer_desc.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
			vertex_buffer_desc.instance_step_rate = 0;
			vertex_buffer_desc.pitch = sizeof(VertexData);

			vertex_attributes[0].buffer_slot = 0;
			vertex_attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
			vertex_attributes[0].location = 0;
			vertex_attributes[0].offset = 0;

			vertex_attributes[1].buffer_slot = 0;
			vertex_attributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
			vertex_attributes[1].location = 1;
			vertex_attributes[1].offset = sizeof(float) * 3;

			pipelinedesc.vertex_input_state.num_vertex_buffers = 1;
			pipelinedesc.vertex_input_state.vertex_buffer_descriptions = &vertex_buffer_desc;
			pipelinedesc.vertex_input_state.num_vertex_attributes = 2;
			pipelinedesc.vertex_input_state.vertex_attributes = (SDL_GPUVertexAttribute *)&vertex_attributes;

			pipelinedesc.props = 0;

			SDL_GPUGraphicsPipeline *pipeline = SDL_CreateGPUGraphicsPipeline(gpu->device, &pipelinedesc);
			if (pipeline == NULL) {
				ecs_enable(world, e, false);
				continue;
			}
			ecs_set(world, e, EgGpuPipeline, {.object = pipeline});
			ecs_dbg("SDL_CreateGPUGraphicsPipeline(%p) -> %p", gpu->device, pipeline);
		}
		ecs_log_pop_1();
	} // END FOR LOOP
	ecs_log_pop_1();
	ecs_log_set_level(0);
}

void EgGpuImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgGpu);
	ecs_set_name_prefix(world, "EgGpu");

	ECS_COMPONENT_DEFINE(world, EgGpuDevice);
	ECS_COMPONENT_DEFINE(world, EgGpuDeviceCreateInfo);
	ECS_COMPONENT_DEFINE(world, EgGpuShaderVertex);
	ECS_COMPONENT_DEFINE(world, EgGpuShaderVertexCreateInfo);
	ECS_COMPONENT_DEFINE(world, EgGpuShaderFragment);
	ECS_COMPONENT_DEFINE(world, EgGpuShaderFragmentCreateInfo);
	ECS_COMPONENT_DEFINE(world, EgGpuPipeline);
	ECS_COMPONENT_DEFINE(world, EgGpuPipelineCreateInfo);

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

	ecs_system(world,
	{.entity = ecs_entity(world, {.name = "System_EgGpuDevice_Create", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_EgGpuDevice_Create,
	.query.terms =
	{
	{.id = ecs_id(EgGpuDeviceCreateInfo), .src.id = EcsSelf},
	{.id = ecs_id(EgGpuDevice), .oper = EcsNot}, // Adds this
	}});

	ecs_system(world,
	{.entity = ecs_entity(world, {.name = "System_EgGpuShaderFragment_Create", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_EgGpuShaderFragment_Create,
	.query.terms =
	{
	{.id = ecs_id(EgGpuDevice), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsIn},
	{.id = ecs_id(EgGpuShaderFragmentCreateInfo), .src.id = EcsSelf},
	{.id = ecs_id(EgGpuShaderFragment), .oper = EcsNot}, // Adds this
	}});

	ecs_system(world,
	{.entity = ecs_entity(world, {.name = "System_EgGpuShaderVertex_Create", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_EgGpuShaderVertex_Create,
	.query.terms =
	{
	{.id = ecs_id(EgGpuDevice), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsIn},
	{.id = ecs_id(EgGpuShaderVertexCreateInfo), .src.id = EcsSelf},
	{.id = ecs_id(EgGpuShaderVertex), .oper = EcsNot}, // Adds this
	}});

	ecs_system(world,
	{.entity = ecs_entity(world, {.name = "System_EgGpuPipeline_Create", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_EgGpuPipeline_Create,
	.query.terms =
	{
	{.id = ecs_id(EgGpuDevice), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsIn},
	{.id = ecs_id(EgGpuPipelineCreateInfo), .src.id = EcsSelf},
	{.id = ecs_id(EgGpuShaderVertex), .trav = EcsDependsOn, .src.id = EcsUp},
	{.id = ecs_id(EgGpuShaderFragment), .trav = EcsDependsOn, .src.id = EcsUp},
	{.id = ecs_id(EgGpuPipeline), .oper = EcsNot}, // Adds this
	}});
}
