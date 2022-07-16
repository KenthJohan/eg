#pragma once

#include "flecs.h"
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

void ecsvk_init(ecs_world_t * world);
void ecsvk_phyiscial(ecs_world_t * world, VkInstance instance, VkSurfaceKHR surface);


#ifdef __cplusplus
}
#endif
