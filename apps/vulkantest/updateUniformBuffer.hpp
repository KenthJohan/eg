#pragma once


#include <vulkan/vulkan.h>
#include "flecs.h"


#ifdef __cplusplus
extern "C" {
#endif


void updateUniformBuffer(VkDevice device, VkExtent2D swapChainExtent, VkDeviceMemory uniformBuffersMemory);


#ifdef __cplusplus
}
#endif
