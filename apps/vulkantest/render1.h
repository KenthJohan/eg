#pragma once
#include "flecs.h"
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

ecs_entity_t render1_init(ecs_world_t * world);
VkSampleCountFlagBits render1_get_max_usable_sample_count(VkPhysicalDevice physicalDevice);
VkPhysicalDevice render1_pick_physical_device(ecs_world_t * world);

#ifdef __cplusplus
}
#endif
