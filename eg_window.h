#pragma once
#include "flecs.h"

#define EG_USRINPUT_ADD(x,c) ((x)[(uint64_t)(c) >> 6] |= (UINT64_C(1) << ((c) & 63)))
#define EG_USRINPUT_DEL(x,c) ((x)[(uint64_t)(c) >> 6] &= ~(UINT64_C(1) << ((c) & 63)))
#define EG_USRINPUT_GET(x,c) (((x)[(uint64_t)(c) >> 6] >> ((c)&63)) & 1)

typedef struct
{
	ecs_string_t title;
	ecs_u64_t userinput[4];
} EgWindow;


extern ECS_COMPONENT_DECLARE(EgWindow);


void FlecsComponentsEgWindowImport(ecs_world_t *world);
