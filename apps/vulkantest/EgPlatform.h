#pragma once
#include "flecs.h"


#ifdef __cplusplus
extern "C" {
#endif

extern ECS_DECLARE(EgPlatformLogVerbose);
extern ECS_DECLARE(EgPlatformLogInfo);
extern ECS_DECLARE(EgPlatformLogWarning);
extern ECS_DECLARE(EgPlatformLogError);

void eg_platform_wait_positive_framebuffer_size(ecs_world_t *world, ecs_entity_t e);
void eg_platform_update();

void EgPlatformImport(ecs_world_t *world);



#ifdef __cplusplus
}
#endif
