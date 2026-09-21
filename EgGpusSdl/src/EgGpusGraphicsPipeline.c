#include "EgGpusGraphicsPipeline.h"
#include <SDL3/SDL_gpu.h>
#include <EgGpus.h>
#include "misc.h"
#include "ecsx.h"

void EgGpusGraphicsPipeline_Create(ecs_iter_t *it)
{
	int32_t loglvl = 0;

	ecs_world_t *world = it->world;

	ecs_trace("System_EgGpuPipeline_Create() count:%i", it->count);
	ecs_log_push_(loglvl);

	for (int i = 0; i < it->count; ++i) {
		ecs_entity_t e = it->entities[i];
		// Entities can be annotated with the Final trait, which prevents using them with IsA relationship.
		ecs_add_id(world, e, EcsFinal);
	}

	EgGpusDevice                     *g = ecs_field_shared(it, EgGpusDevice, 0);                   // shared, parent
	EgGpusGraphicsPipelineCreateInfo *c = ecs_field_self(it, EgGpusGraphicsPipelineCreateInfo, 1); // self
	EcsComponent                     *h = ecs_field_shared(it, EcsComponent, 2);                   // shared
	ecs_entity_t                      k = ecs_field_src(it, 2);                                    // shared

	for (int i = 0; i < it->count; ++i, ++c) {
		ecs_entity_t e = it->entities[i];
		ecs_log(loglvl, "Entity: '%s'", ecs_get_name(world, e));
		ecs_log_push_(loglvl);

		EgGpusShader const *vertex_shader   = ecs_get(world, c->shader_vertex, EgGpusShader);
		EgGpusShader const *fragment_shader = ecs_get(world, c->shader_fragment, EgGpusShader);

		if (!vertex_shader || !fragment_shader) {
			ecs_err("Failed to get shaders");
			ecs_enable(world, e, false);
			continue;
		}

		SDL_GPUColorTargetDescription     color                 = {0};
		SDL_GPUVertexAttribute            attrs[MAX_ATTRIBUTES] = {0};
		SDL_GPUVertexBufferDescription    buf                   = {0};
		SDL_GPUGraphicsPipelineCreateInfo desc                  = {0};

		color.format                                       = SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM;
		desc.target_info.num_color_targets                 = 1;
		desc.target_info.color_target_descriptions         = &color;
		desc.target_info.depth_stencil_format              = SDL_GPU_TEXTUREFORMAT_D16_UNORM;
		desc.target_info.has_depth_stencil_target          = true;
		desc.depth_stencil_state.enable_depth_test         = true;
		desc.depth_stencil_state.enable_depth_write        = true;
		desc.depth_stencil_state.compare_op                = SDL_GPU_COMPAREOP_LESS_OR_EQUAL;
		desc.multisample_state.sample_count                = c->sample_count;
		desc.primitive_type                                = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
		desc.vertex_shader                                 = vertex_shader->object;
		desc.fragment_shader                               = fragment_shader->object;
		buf.slot                                           = 0;
		buf.input_rate                                     = SDL_GPU_VERTEXINPUTRATE_VERTEX;
		buf.instance_step_rate                             = 0;
		buf.pitch                                          = h->size;
		desc.vertex_input_state.num_vertex_buffers         = 1;
		desc.vertex_input_state.vertex_buffer_descriptions = &buf;
		desc.vertex_input_state.vertex_attributes          = (SDL_GPUVertexAttribute *)&attrs;
		desc.vertex_input_state.num_vertex_attributes      = EcsMember_to_SDL_GPUVertexAttribute_array(attrs, world, k);
		desc.props                                         = 0;
		SDL_GPUGraphicsPipeline *pipeline                  = SDL_CreateGPUGraphicsPipeline(g->object, &desc);
		if (pipeline == NULL) {
			ecs_err("SDL_CreateGPUGraphicsPipeline() failed");
			ecs_enable(world, e, false);
			continue;
		}
		ecs_set(world, e, EgGpusGraphicsPipeline, {.object = pipeline, .info_num_vertex_attributes = desc.vertex_input_state.num_vertex_attributes});
		ecs_log(loglvl, "SDL_CreateGPUGraphicsPipeline(%p) -> %p", g->object, pipeline);

		ecs_log_pop_(loglvl);
	} // END FOR LOOP
	ecs_log_pop_(loglvl);
}
