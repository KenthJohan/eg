#pragma once

#include "flecs.h"
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

void EgVkImport(ecs_world_t *world);
void populate_VkPhysicalDevice(ecs_world_t * world, VkInstance instance, VkSurfaceKHR surface);


#ifdef __cplusplus
}
#endif
