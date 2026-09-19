#include "EgGpusShaderVertex.h"
#include <EgGpus.h>
#include <egmisc.h>
#include <ecsx.h>
#include <SDL3/SDL_gpu.h>

void EgGpusShaderVertex_Create(ecs_iter_t *it)
{
	int32_t loglvl = 0;

	EgGpusDevice *device = ecs_field_shared(it, EgGpusDevice, 0);
	EgGpusShaderVertexCreateInfo *ci = ecs_field_self(it, EgGpusShaderVertexCreateInfo, 1);

	for (int i = 0; i < it->count; ++i, ++ci) {
		ecs_log(loglvl, "Creating shader vertex with path: %s", ci->path);

		size_t      source_size = 0;
		char const *source      = eg_file_load_alloc(ci->path, &source_size);

		SDL_GPUShaderCreateInfo info = {};
		info.entrypoint              = "main";
		info.stage                   = SDL_GPU_SHADERSTAGE_VERTEX;
		info.num_uniform_buffers     = 1;
		info.num_storage_buffers     = 0;
		info.num_storage_textures    = 0;
		info.num_samplers            = 0;
		info.code                    = source;
		info.code_size               = source_size;

		// Create the shader
		SDL_GPUShader *shader = SDL_CreateGPUShader((SDL_GPUDevice *)device->object, &info);
		if (!shader) {
			ecs_log(loglvl, "Failed to create shader vertex with path: %s", ci->path);
		}

        ecs_os_free((void *)source);
	}

}
