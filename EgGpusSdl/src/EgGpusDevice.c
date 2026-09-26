#include "EgGpusDevice.h"
#include <SDL3/SDL_gpu.h>
#include <ecsx.h>

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

static void set_doc_from_SDL_GPUDevice(ecs_world_t *world, ecs_entity_t e, SDL_GPUDevice *gpu_device)
{
	char                buf[128];
	char const         *driver  = SDL_GetGPUDeviceDriver(gpu_device);
	SDL_GPUShaderFormat formats = SDL_GetGPUShaderFormats(gpu_device);
	snprintf(buf, 128, "Driver: %s, %08X", driver, formats);
	ecs_doc_set_brief(world, e, buf);
}

void EgGpusDevice_Create(ecs_iter_t *it)
{
	int32_t loglvl = 0;

	ecs_world_t *world = it->world;

	EgGpusDeviceCreateInfo *c = ecs_field_self(it, EgGpusDeviceCreateInfo, 0);

	ecs_log(loglvl, EG_GPUS_LOGTAG "System Sampler_Create() count:%i", it->count);
	ecs_log_push_(loglvl);
	for (int i = 0; i < it->count; ++i, ++c) {
		ecs_entity_t e = it->entities[i];
		ecs_log(loglvl, "Entity: '%s'", ecs_get_name(world, e));
		ecs_log_push_(loglvl);
		{
			SDL_GPUShaderFormat formats = SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXBC | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_METALLIB;

			SDL_GPUDevice *gpu_device = SDL_CreateGPUDevice(formats, true, NULL);
			ecs_log(loglvl, EG_GPUS_LOGTAG "SDL_CreateGPUDevice() -> %p", gpu_device);
			if (gpu_device == NULL) {
				ecs_err(EG_GPUS_LOGTAG "Failed to create GPU device for entity: %s", ecs_get_name(world, e));
				ecs_enable(world, e, false);
				continue;
			}

			EgGpusDevice dev = {
			.object = gpu_device,
			};
			ecs_set_ptr(world, e, EgGpusDevice, &dev);

			set_doc_from_SDL_GPUDevice(world, e, gpu_device);

			// Entities can be annotated with the Final trait, which prevents using them with IsA relationship.
			ecs_add_id(world, e, EcsFinal);
		}
		ecs_log_pop_(loglvl);

	} // END FOR LOOP
	ecs_log_pop_(loglvl);
}
