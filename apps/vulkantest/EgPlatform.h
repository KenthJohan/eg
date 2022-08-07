#pragma once
#include "flecs.h"


#ifdef __cplusplus
extern "C" {
#endif


void eg_platform_wait_positive_framebuffer_size(ecs_world_t *world, ecs_entity_t e);
void eg_platform_update();

void EgPlatformImport(ecs_world_t *world);



#ifdef __cplusplus
}
#endif
