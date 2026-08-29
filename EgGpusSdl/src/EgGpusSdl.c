#include "EgGpusSdl.h"
#include <EgGpus.h>
#include <EgShapes.h>
#include <SDL3/SDL_gpu.h>

void EgGpusDevice_add(ecs_iter_t *it)
{
	EgGpusDevice *device = ecs_field(it, EgGpusDevice, 0);

	for (int i = 0; i < it->count; i++, device++) {
		device->device = NULL;
	}
}

void EgGpusDevice_remove(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;
	EgGpusDevice *device = ecs_field(it, EgGpusDevice, 0);

	for (int i = 0; i < it->count; i++, device++) {
		ecs_entity_t e = it->entities[i];
		if (device->device) {
			ecs_trace("Releasing GPU device: %s", ecs_get_name(world, e));
			SDL_DestroyGPUDevice(device->device);
		}
	}
}

void EgGpusTexture_add(ecs_iter_t *it)
{
	EgGpusTexture *texture = ecs_field(it, EgGpusTexture, 0);

	for (int i = 0; i < it->count; i++, texture++) {
		texture->object = NULL;
	}
}

void EgGpusTexture_remove(ecs_iter_t *it)
{
	ecs_world_t *world = it->world;
	EgGpusTexture *texture = ecs_field(it, EgGpusTexture, 0);

	for (int i = 0; i < it->count; i++, texture++) {
		ecs_entity_t e = it->entities[i];
		ecs_entity_t parent = ecs_get_parent(world, e);
		if (!parent) {
			continue;
		}
		const EgGpusDevice *device = ecs_get(world, parent, EgGpusDevice);
		if (texture->object && device && device->device) {
			ecs_trace("Releasing GPU texture: %s", ecs_get_name(world, e));
			SDL_ReleaseGPUTexture(device->device, texture->object);
		}
	}
}

#define TESTGPU_SUPPORTED_FORMATS (SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXBC | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_METALLIB)

void System_EgGpusDevice_Create(ecs_iter_t *it)
{
	ecs_world_t            *world  = it->world;
	EgGpusDeviceCreateInfo *create = ecs_field(it, EgGpusDeviceCreateInfo, 0);

	ecs_log_set_level(1);
	ecs_trace("System Sampler_Create() count:%i", it->count);
	ecs_log_push_(0);
	for (int i = 0; i < it->count; ++i, ++create) {
		ecs_entity_t e = it->entities[i];
		ecs_trace("Entity: '%s'", ecs_get_name(world, e));
		ecs_log_push_(0);
		{
			SDL_GPUDevice *device = SDL_CreateGPUDevice(TESTGPU_SUPPORTED_FORMATS, true, NULL);
			if (device == NULL) {
				ecs_enable(world, e, false);
				continue;
			}
			EgGpusDevice dev = {
			.device = device,
			};
			ecs_set_ptr(world, e, EgGpusDevice, &dev);
			ecs_trace("SDL_CreateGPUDevice() -> %p", device);
			char buf[128];
			snprintf(buf, 128, "%s", ecs_get_name(world, e));
			ecs_doc_set_name(world, e, buf);
			char const         *driver  = SDL_GetGPUDeviceDriver(device);
			SDL_GPUShaderFormat formats = SDL_GetGPUShaderFormats(device);
			// ecs_doc_set_color(world, e, color);
			snprintf(buf, 128, "Driver: %s, %08X", driver, formats);
			ecs_doc_set_brief(world, e, buf);
			// Entities can be annotated with the Final trait, which prevents using them with IsA relationship.
			ecs_add_id(world, e, EcsFinal);
		}
		ecs_log_pop_(0);

	} // END FOR LOOP
	ecs_log_pop_(0);
	ecs_log_set_level(0);
}

void Observer_EgGpuTexture(ecs_iter_t *it)
{
	if (it->event_id != ecs_id(EgShapesRectangle)) {
		return;
	}

	ecs_world_t       *world = it->world;
	EgShapesRectangle *r     = ecs_field(it, EgShapesRectangle, 0); // self
	EgGpusTexture     *t     = ecs_field(it, EgGpusTexture, 1);     // self
	EgGpusDevice      *g     = ecs_field(it, EgGpusDevice, 2);      // shared
	EgGpusTextureCreateInfo *create = ecs_field(it, EgGpusTextureCreateInfo, 3);
	for (int i = 0; i < it->count; ++i, ++r, ++t) {
		if (r->w < 1.0f || r->h < 1.0f || r->w > UINT32_MAX || r->h > UINT32_MAX) {
			ecs_err("Invalid texture size (%f %f)", r->w, r->h);
			ecs_enable(world, it->entities[i], false);
			continue;
		}
		printf("Changing texture (%s) to size (%f %f)\n", ecs_get_name(world, it->entities[i]), r->w, r->h);
		if (t->object) {
			SDL_ReleaseGPUTexture(g->device, t->object);
		}
		SDL_GPUTextureCreateInfo info = {0};
		info.type                     = SDL_GPU_TEXTURETYPE_2D;
		info.format                   = SDL_GPU_TEXTUREFORMAT_D16_UNORM;
		info.width                    = (uint32_t)r->w;
		info.height                   = (uint32_t)r->h;
		info.layer_count_or_depth     = 1;
		info.num_levels               = 1;
		info.sample_count             = create ? create[i].sample_count : 1;
		info.usage                    = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
		info.props                    = 0;
		t->object                     = SDL_CreateGPUTexture(g->device, &info);
		if (!t->object) {
			ecs_err("Failed to create texture: %s with size (%f %f)", SDL_GetError(), r->w, r->h);
			ecs_enable(world, it->entities[i], false);
			continue;
		}
		SDL_SetGPUTextureName(g->device, t->object, ecs_get_name(world, it->entities[i]));
	} // END FOR LOOP
}

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
	.on_add = EgGpusDevice_add,
	.on_remove = EgGpusDevice_remove,
	});
	ecs_set_hooks(world, EgGpusTexture,
	{
	.on_add = EgGpusTexture_add,
	.on_remove = EgGpusTexture_remove,
	});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity   = ecs_entity(world, {.name = "System_EgGpusDevice_Create"}),
	.callback = System_EgGpusDevice_Create,
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
	.callback    = Observer_EgGpuTexture,
	.events      = {EcsOnSet},
	.query.terms = {
	{.id = ecs_id(EgShapesRectangle)},
	{.id = ecs_id(EgGpusTexture), .inout = EcsInOutFilter},
	{.id = ecs_id(EgGpusDevice), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsInOutFilter},
	{.id = ecs_id(EgGpusTextureCreateInfo), .oper = EcsOptional, .inout = EcsInOutFilter},
	}});
}
