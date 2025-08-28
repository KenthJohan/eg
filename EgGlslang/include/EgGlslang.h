#pragma once
#include <flecs.h>


typedef struct
{
	int dummy;
} EgGlslangCreate;

typedef struct
{
	uint32_t * words;
	int32_t words_size;
} EgGlslangProgram;

extern ECS_COMPONENT_DECLARE(EgGlslangCreate);
extern ECS_COMPONENT_DECLARE(EgGlslangProgram);

void EgGlslangImport(ecs_world_t *world);
