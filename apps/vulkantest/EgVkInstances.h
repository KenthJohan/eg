#pragma once

#include "flecs.h"
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

extern ECS_COMPONENT_DECLARE(EgVkInstances);
void createInstance1(ecs_world_t * world, ecs_entity_t e);
void EgVkInstancesImport(ecs_world_t *world);


#ifdef __cplusplus
}
#endif
