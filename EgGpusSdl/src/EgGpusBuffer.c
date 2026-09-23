#include "EgGpusBuffer.h"
#include <SDL3/SDL_gpu.h>
#include <ecsx.h>

void EgGpusBuffer_add(ecs_iter_t *it)
{
	EgGpusBuffer *buffer = ecs_field(it, EgGpusBuffer, 0);

	for (int i = 0; i < it->count; ++i, ++buffer) {
		buffer->object = NULL;
	}
}

void EgGpusBuffer_remove(ecs_iter_t *it)
{
	EgGpusBuffer *buffer = ecs_field(it, EgGpusBuffer, 0);

	for (int i = 0; i < it->count; ++i, ++buffer) {
		ecs_entity_t parent = ecs_get_parent(it->world, it->entities[i]);
		const EgGpusDevice *device = parent ? ecs_get(it->world, parent, EgGpusDevice) : NULL;
		if (buffer->object && device && device->object) {
			SDL_ReleaseGPUBuffer(device->object, buffer->object);
		}
	}
}

static SDL_GPUBufferUsageFlags EgGpusBuffer_usage(EgGpusBufferUsage usage)
{
	switch (usage) {
	case EgGpusBufferUsageVertex:
		return SDL_GPU_BUFFERUSAGE_VERTEX;
	case EgGpusBufferUsageIndex:
		return SDL_GPU_BUFFERUSAGE_INDEX;
	case EgGpusBufferUsageUniform:
		return SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
	}
	return 0;
}

void EgGpusBuffer_Create(ecs_iter_t *it)
{
	EgGpusDevice *device = ecs_field_shared(it, EgGpusDevice, 0);
	EgGpusBufferCreateInfo *create = ecs_field_self(it, EgGpusBufferCreateInfo, 1);

	for (int i = 0; i < it->count; ++i, ++create) {
		if (create->size == 0) {
			ecs_enable(it->world, it->entities[i], false);
			continue;
		}

		SDL_GPUBufferCreateInfo info = {
		.usage = EgGpusBuffer_usage(create->usage),
		.size = create->size,
		};
		SDL_GPUBuffer *object = SDL_CreateGPUBuffer(device->object, &info);
		if (!object) {
			ecs_enable(it->world, it->entities[i], false);
			continue;
		}
		ecs_set(it->world, it->entities[i], EgGpusBuffer, {.object = object});
	}
}
