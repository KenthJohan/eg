#pragma once

#include "flecs.h"
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

void populate_VkPhysicalDevice(ecs_world_t * world, VkInstance instance, VkSurfaceKHR surface);
//void createInstance1(ecs_world_t * world, ecs_entity_t e);
void EgVkSystemsImport(ecs_world_t *world);





#ifdef __cplusplus
}
#endif
