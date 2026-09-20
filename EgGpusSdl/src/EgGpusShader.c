#include "EgGpusShader.h"
#include <EgGpus.h>
#include <EgFs.h>
#include <ecsx.h>
#include <SDL3/SDL_gpu.h>

void EgGpusShader_Create(ecs_iter_t *it)
{
	EgGpusDevice           *device  = ecs_field_shared(it, EgGpusDevice, 0);
	EgGpusShaderCreateInfo *ci      = ecs_field_self(it, EgGpusShaderCreateInfo, 1);
	EgFsContent            *content = ecs_field_self(it, EgFsContent, 2);

	for (int i = 0; i < it->count; ++i, ++ci) {
		if (content[i].data == NULL || content[i].size == 0) {
			ecs_err("Cannot create GPU shader with empty content");
			ecs_enable(it->world, it->entities[i], false);
			continue;
		}

		SDL_GPUShaderCreateInfo info = {};
		info.entrypoint              = "main";
		info.stage                   = (SDL_GPUShaderStage)ci->stage;
		info.format                  = SDL_GPU_SHADERFORMAT_SPIRV;
		info.num_uniform_buffers     = 1;
		info.num_storage_buffers     = 0;
		info.num_storage_textures    = 0;
		info.num_samplers            = 0;
		info.code                    = content[i].data;
		info.code_size               = content[i].size;

		SDL_GPUShader *shader = SDL_CreateGPUShader((SDL_GPUDevice *)device->object, &info);
		if (!shader) {
			ecs_err("Failed to create GPU shader");
			ecs_enable(it->world, it->entities[i], false);
			continue;
		}
		ecs_set(it->world, it->entities[i], EgGpusShader, {.object = shader});
	}
}