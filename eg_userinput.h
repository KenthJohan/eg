#pragma once
#include "flecs.h"

// If SDL is used as backend
#if 1
#include "eg_key_sdl.h"
#endif


typedef struct
{
	ecs_u64_t keyboard[4];
} EgUserinput;

extern ECS_COMPONENT_DECLARE(EgUserinput);


void FlecsComponentsEgUserinputImport(ecs_world_t *world);
