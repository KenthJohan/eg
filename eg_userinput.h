#pragma once
#include "flecs.h"

// If SDL is used as backend
#if 1
#include "eg_key_sdl.h"
#endif

#define EG_NUM_KEYS 512
#define EG_NUM_KEYS64 (EG_NUM_KEYS/64)

typedef struct
{
	ecs_u64_t keyboard[4];
} EgUserinput;

extern ECS_COMPONENT_DECLARE(EgUserinput);


void FlecsComponentsEgUserinputImport(ecs_world_t *world);
