#include "EgGpusShader.h"
#include <EgGpus.h>
#include <EgFs.h>
#include <ecsx.h>
#include <SDL3/SDL_gpu.h>

void EgGpusShader_Create(ecs_iter_t *it)
{
	EgGpusDevice           *device  = ecs_field_shared(it, EgGpusDevice, 0);
	EgGpusShaderCreateInfo *ci      = ecs_field_self(it, EgGpusShaderCreateInfo, 1);
	EgFsContent            *content = ecs_field_shared(it, EgFsContent, 2);

	for (int i = 0; i < it->count; ++i, ++ci) {
		ecs_entity_t e = it->entities[i];

		if (content->data == NULL || content->size == 0) {
			ecs_err("Cannot create GPU shader with empty content");
			ecs_enable(it->world, e, false);
			continue;
		}

		SDL_GPUShaderCreateInfo info = {};
		info.entrypoint              = "main";
		info.stage                   = (SDL_GPUShaderStage)ci->stage;
		info.format                  = SDL_GPU_SHADERFORMAT_SPIRV;
		// Vertex stage uses a uniform buffer (scale/translate); fragment stage samples a texture.
		info.num_uniform_buffers  = ci->stage == EgGpusShaderStageVertex ? 1 : 0;
		info.num_storage_buffers  = 0;
		info.num_storage_textures = 0;
		info.num_samplers         = ci->stage == EgGpusShaderStageFragment ? 1 : 0;
		info.code                 = content->data;
		info.code_size            = content->size;

		SDL_GPUShader *shader = SDL_CreateGPUShader((SDL_GPUDevice *)device->object, &info);
		if (!shader) {
			ecs_err("Failed to create GPU shader");
			ecs_enable(it->world, e, false);
			continue;
		}

		if (ci->stage == EgGpusShaderStageVertex) {
			ecs_set(it->world, e, EgGpusShaderVertex, {.object = shader});
		} else if (ci->stage == EgGpusShaderStageFragment) {
			ecs_set(it->world, e, EgGpusShaderFragment, {.object = shader});
		}
	}
}

static void EgGpusShader_release_object(ecs_world_t *world, ecs_entity_t e, void *shader_object)
{
	if (!shader_object) {
		ecs_err(EG_GPUS_LOGTAG "Shader (%s) has no shader object", ecs_get_name(world, e));
		return;
	}
	ecs_entity_t parent = ecs_get_parent(world, e);
	if (!parent) {
		ecs_err(EG_GPUS_LOGTAG "Shader (%s) has no parent device", ecs_get_name(world, e));
		return;
	}
	const EgGpusDevice *device = ecs_get(world, parent, EgGpusDevice);
	if (!device || !device->object) {
		ecs_err(EG_GPUS_LOGTAG "Shader (%s) has no valid parent device object", ecs_get_name(world, e));
		return;
	}
	// This check is now redundant because it's included in the previous check.
	ecs_log(0, EG_GPUS_LOGTAG "Releasing GPU shader: %s", ecs_get_name(world, e));
	SDL_ReleaseGPUShader((SDL_GPUDevice *)device->object, (SDL_GPUShader *)shader_object);
}

void EgGpusShaderVertex_remove(ecs_iter_t *it)
{
	EgGpusShaderVertex *s = ecs_field(it, EgGpusShaderVertex, 0);
	for (int i = 0; i < it->count; ++i, ++s) {
		EgGpusShader_release_object(it->world, it->entities[i], s->object);
	}
}

void EgGpusShaderFragment_remove(ecs_iter_t *it)
{
	EgGpusShaderFragment *s = ecs_field(it, EgGpusShaderFragment, 0);
	for (int i = 0; i < it->count; ++i, ++s) {
		EgGpusShader_release_object(it->world, it->entities[i], s->object);
	}
}
