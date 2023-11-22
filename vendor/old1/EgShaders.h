#pragma once
#include "flecs.h"



typedef struct
{
	ecs_f32_t width;
	ecs_f32_t height;
} EgShader;

extern ECS_COMPONENT_DECLARE(EgRectangleF32);


void EgShadersImport(ecs_world_t *world);
