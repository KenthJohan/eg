#pragma once
#include "flecs.h"


#ifdef __cplusplus
extern "C" {
#endif

void platform_populate_required_extension_names(ecs_world_t * world);


void EgPlatformImport(ecs_world_t *world);


#ifdef __cplusplus
}
#endif
