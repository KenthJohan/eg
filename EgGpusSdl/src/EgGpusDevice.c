#include "EgGpusDevice.h"
#include <SDL3/SDL_gpu.h>

void EgGpusDevice_add(ecs_iter_t *it)
{
	EgGpusDevice *device = ecs_field(it, EgGpusDevice, 0);

	for (int i = 0; i < it->count; i++, device++) {
		device->object = NULL;
	}
}

void EgGpusDevice_remove(ecs_iter_t *it)
{
	int32_t loglvl = 0;

	ecs_world_t *world = it->world;

	EgGpusDevice *device = ecs_field(it, EgGpusDevice, 0);

	for (int i = 0; i < it->count; i++, device++) {
		ecs_entity_t e = it->entities[i];
		if (device->object) {
			ecs_log(loglvl, EG_GPUS_LOGTAG "Releasing GPU device: %s", ecs_get_name(world, e));
			SDL_DestroyGPUDevice(device->object);
		}
	}
}

#define TESTGPU_SUPPORTED_FORMATS (SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXBC | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_METALLIB)

void EgGpusDevice_Create(ecs_iter_t *it)
{
	int32_t loglvl = 0;

	ecs_world_t *world = it->world;

	EgGpusDeviceCreateInfo *create = ecs_field(it, EgGpusDeviceCreateInfo, 0);

	ecs_log_set_level(1);
	ecs_log(loglvl, EG_GPUS_LOGTAG "System Sampler_Create() count:%i", it->count);
	ecs_log_push_(0);
	for (int i = 0; i < it->count; ++i, ++create) {
		ecs_entity_t e = it->entities[i];
		ecs_trace("Entity: '%s'", ecs_get_name(world, e));
		ecs_log_push_(0);
		{
			SDL_GPUDevice *gpu_device = SDL_CreateGPUDevice(TESTGPU_SUPPORTED_FORMATS, true, NULL);
			if (gpu_device == NULL) {
				ecs_enable(world, e, false);
				continue;
			}
			EgGpusDevice dev = {
			.object = gpu_device,
			};
			ecs_set_ptr(world, e, EgGpusDevice, &dev);
			ecs_log(loglvl, EG_GPUS_LOGTAG "SDL_CreateGPUDevice() -> %p", gpu_device);
			char buf[128];
			snprintf(buf, 128, "%s", ecs_get_name(world, e));
			ecs_doc_set_name(world, e, buf);
			char const         *driver  = SDL_GetGPUDeviceDriver(gpu_device);
			SDL_GPUShaderFormat formats = SDL_GetGPUShaderFormats(gpu_device);
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
