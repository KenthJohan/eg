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
	EgGpusShaderVertex               *v = ecs_field_shared(it, EgGpusShaderVertex, 3);             // shared
	EgGpusShaderFragment             *f = ecs_field_shared(it, EgGpusShaderFragment, 4);           // shared

	for (int i = 0; i < it->count; ++i, ++c) {
		ecs_entity_t e = it->entities[i];
		ecs_log(loglvl, "Entity: '%s'", ecs_get_name(world, e));
		ecs_log_push_(loglvl);

		SDL_GPUVertexBufferDescription buf = {0};
		buf.slot                           = 0;
		buf.input_rate                     = SDL_GPU_VERTEXINPUTRATE_VERTEX;
		buf.instance_step_rate             = 0;
		buf.pitch                          = h->size;

		SDL_GPUColorTargetDescription color = {0};
		color.format                        = SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM;

		SDL_GPUVertexAttribute attrs[MAX_ATTRIBUTES] = {0};
		Uint32                 attrs_count           = EcsMember_to_SDL_GPUVertexAttribute_array(attrs, world, k);

		SDL_GPUGraphicsPipelineCreateInfo desc             = {0};
		desc.target_info.num_color_targets                 = 1;
		desc.target_info.color_target_descriptions         = &color;
		desc.target_info.depth_stencil_format              = SDL_GPU_TEXTUREFORMAT_D16_UNORM;
		desc.target_info.has_depth_stencil_target          = true;
		desc.depth_stencil_state.enable_depth_test         = true;
		desc.depth_stencil_state.enable_depth_write        = true;
		desc.depth_stencil_state.compare_op                = SDL_GPU_COMPAREOP_LESS_OR_EQUAL;
		desc.multisample_state.sample_count                = EgGpusSdl_SampleCountToEnum(c->sample_count);
		desc.primitive_type                                = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
		desc.vertex_shader                                 = v->object;
		desc.fragment_shader                               = f->object;
		desc.vertex_input_state.num_vertex_buffers         = 1;
		desc.vertex_input_state.vertex_buffer_descriptions = &buf;
		desc.vertex_input_state.vertex_attributes          = attrs;
		desc.vertex_input_state.num_vertex_attributes      = attrs_count;
		desc.props                                         = 0;

		SDL_GPUGraphicsPipeline *pipeline = SDL_CreateGPUGraphicsPipeline(g->object, &desc);
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

void EgGpusGraphicsPipeline_remove(ecs_iter_t *it)
{
	int32_t loglvl = 0;

	ecs_world_t *world = it->world;

	EgGpusGraphicsPipeline *p = ecs_field(it, EgGpusGraphicsPipeline, 0);

	for (int i = 0; i < it->count; ++i, ++p) {
		ecs_entity_t e = it->entities[i];

		ecs_entity_t parent = ecs_get_parent(world, e);
		if (!parent) {
			continue;
		}
		const EgGpusDevice *device = ecs_get(world, parent, EgGpusDevice);
		if (p->object && device && device->object) {
			ecs_log(loglvl, EG_GPUS_LOGTAG "Releasing GPU pipeline: %s", ecs_get_name(world, e));
			SDL_ReleaseGPUGraphicsPipeline(device->object, p->object);
		}
	}
}
