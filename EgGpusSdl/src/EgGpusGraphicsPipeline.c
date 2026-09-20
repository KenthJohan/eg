#include "EgGpusGraphicsPipeline.h"
#include <SDL3/SDL_gpu.h>
#include <EgGpus.h>
#include "misc.h"
#include "ecsx.h"

void EgGpusGraphicsPipeline_Create(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;
	ecs_log_set_level(0);
	ecs_trace("System_EgGpuPipeline_Create() count:%i", it->count);
	ecs_log_push_(0);

	for (int i = 0; i < it->count; ++i) {
		ecs_entity_t e = it->entities[i];
		// Entities can be annotated with the Final trait, which prevents using them with IsA relationship.
		ecs_add_id(world, e, EcsFinal);
	}

	EgGpusDevice                     *gpu                        = ecs_field_shared(it, EgGpusDevice, 0);                   // shared, parent
	EgGpusGraphicsPipelineCreateInfo *field_info                 = ecs_field_self(it, EgGpusGraphicsPipelineCreateInfo, 1); // self
	EcsComponent                     *field_component            = ecs_field_shared(it, EcsComponent, 2);                   // shared
	ecs_entity_t                      field_component_src_entity = ecs_field_src(it, 2);                                    // shared

	for (int i = 0; i < it->count; ++i, ++field_info) {
		ecs_entity_t e = it->entities[i];
		ecs_trace("Entity: '%s'", ecs_get_name(world, e));
		ecs_log_push_(0);
		{
			SDL_GPUShader *vertex_shader = NULL;
			SDL_GPUShader *fragment_shader = NULL;
			for (int dependency_index = 0;; ++dependency_index) {
				ecs_entity_t dependency = ecs_get_target(world, e, EcsDependsOn, dependency_index);
				if (dependency == 0) {
					break;
				}
				const EgGpusShaderCreateInfo *shader_info = ecs_get(world, dependency, EgGpusShaderCreateInfo);
				const EgGpusShader *shader = ecs_get(world, dependency, EgGpusShader);
				if (shader_info == NULL || shader == NULL) {
					continue;
				}
				if (shader_info->stage == EgGpusShaderStageVertex) {
					vertex_shader = shader->object;
				} else if (shader_info->stage == EgGpusShaderStageFragment) {
					fragment_shader = shader->object;
				}
			}
			if (vertex_shader == NULL || fragment_shader == NULL) {
				ecs_err("Graphics pipeline requires vertex and fragment shaders");
				ecs_enable(world, e, false);
				continue;
			}
			SDL_GPUColorTargetDescription     color_target_desc                 = {0};
			SDL_GPUVertexAttribute            vertex_attributes[MAX_ATTRIBUTES] = {0};
			SDL_GPUVertexBufferDescription    vertex_buffer_desc                = {0};
			SDL_GPUGraphicsPipelineCreateInfo pipelinedesc                      = {0};
			// color_target_desc.format = SDL_GetGPUSwapchainTextureFormat(gpu->device, state->windows[0]);
			color_target_desc.format                           = SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM;
			pipelinedesc.target_info.num_color_targets         = 1;
			pipelinedesc.target_info.color_target_descriptions = &color_target_desc;
			pipelinedesc.target_info.depth_stencil_format      = SDL_GPU_TEXTUREFORMAT_D16_UNORM;
			pipelinedesc.target_info.has_depth_stencil_target  = true;

			pipelinedesc.depth_stencil_state.enable_depth_test  = true;
			pipelinedesc.depth_stencil_state.enable_depth_write = true;
			pipelinedesc.depth_stencil_state.compare_op         = SDL_GPU_COMPAREOP_LESS_OR_EQUAL;

			pipelinedesc.multisample_state.sample_count = field_info[i].sample_count;

			pipelinedesc.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

			pipelinedesc.vertex_shader   = vertex_shader;
			pipelinedesc.fragment_shader = fragment_shader;

			vertex_buffer_desc.slot               = 0;
			vertex_buffer_desc.input_rate         = SDL_GPU_VERTEXINPUTRATE_VERTEX;
			vertex_buffer_desc.instance_step_rate = 0;
			vertex_buffer_desc.pitch              = field_component->size;

			/*
			vertex_attributes[0].location = 0;
			vertex_attributes[1].location = 0;
			vertex_attributes[0].buffer_slot = 0;
			vertex_attributes[1].buffer_slot = 0;
			vertex_attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
			vertex_attributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
			vertex_attributes[0].offset = 0;
			vertex_attributes[1].offset = sizeof(float) * 3;
			*/

			uint32_t info_num_vertex_attributes = EcsMember_to_SDL_GPUVertexAttribute_array(
			vertex_attributes, world, field_component_src_entity);

			pipelinedesc.vertex_input_state.num_vertex_buffers         = 1;
			pipelinedesc.vertex_input_state.vertex_buffer_descriptions = &vertex_buffer_desc;

			pipelinedesc.vertex_input_state.vertex_attributes     = (SDL_GPUVertexAttribute *)&vertex_attributes;
			pipelinedesc.vertex_input_state.num_vertex_attributes = info_num_vertex_attributes;

			pipelinedesc.props = 0;

			SDL_GPUGraphicsPipeline *pipeline = SDL_CreateGPUGraphicsPipeline(gpu->object, &pipelinedesc);
			if (pipeline == NULL) {
				ecs_err("SDL_CreateGPUGraphicsPipeline() failed");
				ecs_enable(world, e, false);
				continue;
			}
			ecs_set(world, e, EgGpusGraphicsPipeline, {.object = pipeline, .info_num_vertex_attributes = info_num_vertex_attributes});
			ecs_trace("SDL_CreateGPUGraphicsPipeline(%p) -> %p", gpu->object, pipeline);
		}
		ecs_log_pop_(0);
	} // END FOR LOOP
	ecs_log_pop_(0);
	ecs_log_set_level(0);
}
