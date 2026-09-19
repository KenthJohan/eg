#include "EgGpusTexture.h"
#include "EgGpusDevice.h"
#include <SDL3/SDL_gpu.h>
#include <EgShapes.h>

void EgGpusTexture_add(ecs_iter_t *it)
{
	EgGpusTexture *texture = ecs_field(it, EgGpusTexture, 0);

	for (int i = 0; i < it->count; i++, texture++) {
		texture->object = NULL;
	}
}

void EgGpusTexture_remove(ecs_iter_t *it)
{
    int32_t loglvl = 0;

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
			ecs_log(loglvl, EG_GPUS_LOGTAG "Releasing GPU texture: %s", ecs_get_name(world, e));
			SDL_ReleaseGPUTexture(device->device, texture->object);
		}
	}
}

void EgGpusTexture_Observer(ecs_iter_t *it)
{
    int32_t loglvl = 0;

	ecs_world_t *world = it->world;
	if (it->event_id != ecs_id(EgShapesRectangle)) {
		return;
	}

	EgShapesRectangle       *r      = ecs_field(it, EgShapesRectangle, 0); // self
	EgGpusTexture           *t      = ecs_field(it, EgGpusTexture, 1);     // self
	EgGpusDevice            *g      = ecs_field(it, EgGpusDevice, 2);      // shared
	EgGpusTextureCreateInfo *create = ecs_field(it, EgGpusTextureCreateInfo, 3);

	for (int i = 0; i < it->count; ++i, ++r, ++t) {
		ecs_entity_t e = it->entities[i];

		if (r->w < 1.0f || r->h < 1.0f || r->w > UINT32_MAX || r->h > UINT32_MAX) {
			ecs_log(loglvl, EG_GPUS_LOGTAG "Invalid texture size (%f %f)", r->w, r->h);
			ecs_enable(world, it->entities[i], false);
			continue;
		}
		ecs_log(loglvl, EG_GPUS_LOGTAG "Changing texture (%s) to size (%f %f)\n", ecs_get_name(world, e), r->w, r->h);
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
			ecs_log(loglvl, EG_GPUS_LOGTAG "Failed to create texture (%s): %s with size (%f %f)", ecs_get_name(world, e), SDL_GetError(), r->w, r->h);
			ecs_enable(world, e, false);
			continue;
		}
		SDL_SetGPUTextureName(g->device, t->object, ecs_get_name(world, e));
	} // END FOR LOOP
}
