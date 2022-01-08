#pragma once
#include "flecs.h"


typedef struct
{
	ecs_string_t title;
} EgWindow;


extern ECS_COMPONENT_DECLARE(EgWindow);


void FlecsComponentsEgWindowImport(ecs_world_t *world);
