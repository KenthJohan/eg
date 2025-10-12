#pragma once
#include <flecs.h>


typedef struct
{
	ecs_i32_t stage; // 0=vert, 4=frag, ...
} EgGlslangCreate;

typedef struct
{
	uint32_t * words;
	int32_t words_size;
} EgGlslangProgram;

extern ECS_COMPONENT_DECLARE(EgGlslangCreate);
extern ECS_COMPONENT_DECLARE(EgGlslangProgram);

void EgGlslangImport(ecs_world_t *world);
