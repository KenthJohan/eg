#include "EgGpusShaderFragment.h"
#include <EgGpus.h>
#include <egmisc.h>
#include <ecsx.h>
#include <SDL3/SDL_gpu.h>

void EgGpusShaderFragment_Create(ecs_iter_t *it)
{
	EgGpusDevice *device = ecs_field_shared(it, EgGpusDevice, 0);
	EgGpusShaderFragmentCreateInfo *ci = ecs_field_self(it, EgGpusShaderFragmentCreateInfo, 1);

	for (int i = 0; i < it->count; i++, ++ci) {
		size_t source_size = 0;
		char const *source = eg_file_load_alloc(ci->path, &source_size);

		SDL_GPUShaderCreateInfo info = {};
		info.code = source;
		info.code_size = source_size;
		info.entrypoint = "main";
		info.format = SDL_GPU_SHADERFORMAT_SPIRV;
		info.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;

		SDL_GPUShader *shader = SDL_CreateGPUShader((SDL_GPUDevice *)device->object, &info);
		if (!shader) {
			ecs_log(0, "Failed to create shader fragment with path: %s", ci->path);
			ecs_os_free((void *)source);
			continue;
		}
		ecs_set(it->world, it->entities[i], EgGpusShaderFragment, {.object = shader});
		ecs_os_free((void *)source);
	}
}
