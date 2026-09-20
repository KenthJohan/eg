#pragma once
#include <EgFs.h>
#include <EgGpus.h>
#include <flecs.h>

#ifndef EGSPIRV_API
#define EGSPIRV_API
#endif

typedef struct {
	int32_t stage;
} EgSpirvReflect;

typedef struct {
	int32_t      base_type;
	ecs_entity_t type;
	uint32_t     vector_size;
	uint32_t     bit_width;
} EgSpirvShaderInput;

extern ECS_COMPONENT_DECLARE(EgSpirvReflect);
extern ECS_COMPONENT_DECLARE(EgSpirvShaderInput);
extern ecs_entity_t EgSpirvBaseType;

void EgSpirvImport(ecs_world_t *world);
