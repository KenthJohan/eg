#pragma once
#include "flecs.h"
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif


void createTextureImage
(
ecs_world_t * world,
VkPhysicalDevice physicalDevice, VkDevice device,
VkCommandPool commandPool, VkQueue graphicsQueue,
VkImage * textureImage, uint32_t * mipLevels, VkDeviceMemory * textureImageMemory
);


#ifdef __cplusplus
}
#endif
