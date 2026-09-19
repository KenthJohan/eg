#pragma once
#include <EgGpus.h>
#include <flecs.h>

#ifndef EGSPIRV_API
#define EGSPIRV_API
#endif

typedef struct {
	char const *path;
	int32_t stage;
} EgSpirvShaderCreateInfo;

typedef struct {
	uint32_t *words;
	uint32_t word_count;
} EgSpirvShader;

typedef struct {
	int32_t base_type;
	uint32_t vector_size;
	uint32_t bit_width;
} EgSpirvShaderInput;

extern ECS_COMPONENT_DECLARE(EgSpirvShaderCreateInfo);
extern ECS_COMPONENT_DECLARE(EgSpirvShader);
extern ECS_COMPONENT_DECLARE(EgSpirvShaderInput);

void EgSpirvImport(ecs_world_t *world);
