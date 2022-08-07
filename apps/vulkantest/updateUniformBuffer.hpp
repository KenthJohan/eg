#pragma once


#include <vulkan/vulkan.h>
#include "flecs.h"
#include "types.hpp"


#ifdef __cplusplus
extern "C" {
#endif


void updateUniformBuffer(VkDevice device, VkExtent2D swapChainExtent, VkDeviceMemory uniformBuffersMemory[], uint32_t currentImage);


#ifdef __cplusplus
}
#endif
