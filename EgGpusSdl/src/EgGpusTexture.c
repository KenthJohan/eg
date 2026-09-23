#include "EgGpusTexture.h"
#include "EgGpusDevice.h"
#include "misc.h"
#include <SDL3/SDL_gpu.h>
#include <EgShapes.h>
#include <ecsx.h>

static SDL_GPUTextureFormat EgGpusTexture_format(EgGpusTextureFormat format)
{
	return format == EgGpusTextureFormatRgba8Unorm ? SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM : SDL_GPU_TEXTUREFORMAT_D16_UNORM;
}

static SDL_GPUTextureUsageFlags EgGpusTexture_usage(EgGpusTextureUsage usage)
{
	return usage == EgGpusTextureUsageSampled ? SDL_GPU_TEXTUREUSAGE_SAMPLER : SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
}

void EgGpusTexture_add(ecs_iter_t *it)
{
	EgGpusTexture *t = ecs_field(it, EgGpusTexture, 0);

	for (int i = 0; i < it->count; ++i, ++t) {
		t->object = NULL;
	}
}

void EgGpusTexture_remove(ecs_iter_t *it)
{
	int32_t loglvl = 0;

	ecs_world_t *world = it->world;

	EgGpusTexture *t = ecs_field(it, EgGpusTexture, 0);

	for (int i = 0; i < it->count; ++i, ++t) {
		ecs_entity_t e = it->entities[i];

		ecs_entity_t parent = ecs_get_parent(world, e);
		if (!parent) {
			continue;
		}
		const EgGpusDevice *device = ecs_get(world, parent, EgGpusDevice);
		if (t->object && device && device->object) {
			ecs_log(loglvl, EG_GPUS_LOGTAG "Releasing GPU texture: %s", ecs_get_name(world, e));
			SDL_ReleaseGPUTexture(device->object, t->object);
		}
	}
}

void EgGpusTexture_Observer(ecs_iter_t *it)
{
	int32_t loglvl = -1;

	ecs_world_t *world = it->world;

	EgShapesRectangle       *r = ecs_field_self(it, EgShapesRectangle, 0);
	EgGpusTexture           *t = ecs_field_self(it, EgGpusTexture, 1);
	EgGpusDevice            *g = ecs_field_shared(it, EgGpusDevice, 2);
	EgGpusTextureCreateInfo *c = ecs_field_self(it, EgGpusTextureCreateInfo, 3);

	for (int i = 0; i < it->count; ++i, ++r, ++t, ++c) {
		ecs_entity_t e = it->entities[i];

		if (r->w < 1.0f || r->h < 1.0f || r->w > UINT32_MAX || r->h > UINT32_MAX) {
			ecs_log(loglvl, EG_GPUS_LOGTAG "Invalid texture size (%f %f)", r->w, r->h);
			ecs_enable(world, it->entities[i], false);
			continue;
		}
		ecs_log(loglvl, EG_GPUS_LOGTAG "Changing texture (%s) to size (%f %f)\n", ecs_get_name(world, e), r->w, r->h);
		if (t->object) {
			SDL_ReleaseGPUTexture(g->object, t->object);
		}
		SDL_GPUTextureCreateInfo info = {0};
		info.type                     = SDL_GPU_TEXTURETYPE_2D;
		info.format                   = EgGpusTexture_format(c->format);
		info.width                    = (uint32_t)r->w;
		info.height                   = (uint32_t)r->h;
		info.layer_count_or_depth     = 1;
		info.num_levels               = 1;
		info.sample_count             = EgGpusSdl_SampleCountToEnum(c->sample_count);
		info.usage                    = EgGpusTexture_usage(c->usage);
		info.props                    = 0;
		t->object                     = SDL_CreateGPUTexture(g->object, &info);
		if (!t->object) {
			ecs_log(loglvl, EG_GPUS_LOGTAG "Failed to create texture (%s): %s with size (%f %f)", ecs_get_name(world, e), SDL_GetError(), r->w, r->h);
			ecs_enable(world, e, false);
			continue;
		}
		SDL_SetGPUTextureName(g->object, t->object, ecs_get_name(world, e));
	} // END FOR LOOP
}
