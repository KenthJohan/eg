#include "EgGpusSampler.h"
#include <SDL3/SDL_gpu.h>
#include <ecsx.h>

void EgGpusSampler_add(ecs_iter_t *it)
{
	EgGpusSampler *sampler = ecs_field(it, EgGpusSampler, 0);

	for (int i = 0; i < it->count; ++i, ++sampler) {
		sampler->object = NULL;
	}
}

void EgGpusSampler_remove(ecs_iter_t *it)
{
	EgGpusSampler *sampler = ecs_field(it, EgGpusSampler, 0);

	for (int i = 0; i < it->count; ++i, ++sampler) {
		ecs_entity_t parent = ecs_get_parent(it->world, it->entities[i]);
		const EgGpusDevice *device = parent ? ecs_get(it->world, parent, EgGpusDevice) : NULL;
		if (sampler->object && device && device->object) {
			SDL_ReleaseGPUSampler(device->object, sampler->object);
		}
	}
}

static SDL_GPUFilter EgGpusSampler_filter(EgGpusSamplerFilter filter)
{
	return filter == EgGpusSamplerFilterLinear ? SDL_GPU_FILTER_LINEAR : SDL_GPU_FILTER_NEAREST;
}

static SDL_GPUSamplerAddressMode EgGpusSampler_address_mode(EgGpusSamplerAddressMode mode)
{
	return mode == EgGpusSamplerAddressModeRepeat ? SDL_GPU_SAMPLERADDRESSMODE_REPEAT : SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
}

void EgGpusSampler_Create(ecs_iter_t *it)
{
	EgGpusDevice *device = ecs_field_shared(it, EgGpusDevice, 0);
	EgGpusSamplerCreateInfo *create = ecs_field_self(it, EgGpusSamplerCreateInfo, 1);

	for (int i = 0; i < it->count; ++i, ++create) {
		SDL_GPUSamplerCreateInfo info = {0};
		info.min_filter = EgGpusSampler_filter(create->min_filter);
		info.mag_filter = EgGpusSampler_filter(create->mag_filter);
		info.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
		info.address_mode_u = EgGpusSampler_address_mode(create->address_mode_u);
		info.address_mode_v = EgGpusSampler_address_mode(create->address_mode_v);
		info.address_mode_w = EgGpusSampler_address_mode(create->address_mode_w);

		SDL_GPUSampler *object = SDL_CreateGPUSampler(device->object, &info);
		if (!object) {
			ecs_enable(it->world, it->entities[i], false);
			continue;
		}
		ecs_set(it->world, it->entities[i], EgGpusSampler, {.object = object});
	}
}
