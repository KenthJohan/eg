#pragma once
#include "flecs.h"
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif



#define MAX_FRAMES_IN_FLIGHT 2

ecs_entity_t render1_init(ecs_world_t * world);
VkSampleCountFlagBits render1_get_max_usable_sample_count(VkPhysicalDevice physicalDevice);
VkPhysicalDevice render1_pick_physical_device(ecs_world_t * world);


VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);
void endSingleTimeCommands(VkDevice device, VkCommandBuffer commandBuffer, VkQueue graphicsQueue, VkCommandPool commandPool);

void copyBufferToImage(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
void generateMipmaps(ecs_world_t * world, VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
void transitionImageLayout(ecs_world_t * world, VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
uint32_t findMemoryType(ecs_world_t * world, VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
void createBuffer(ecs_world_t * world, VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer * buffer, VkDeviceMemory * bufferMemory);
void createImage
(
ecs_world_t * world,
VkPhysicalDevice physicalDevice, VkDevice device,
uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples,
VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
VkMemoryPropertyFlags properties,
VkImage * image, VkDeviceMemory * imageMemory
);
void createDescriptorSetLayout(ecs_world_t * world, VkDevice device, VkDescriptorSetLayout * descriptorSetLayout);


void copyBuffer(VkDevice device, VkQueue graphicsQueue, VkCommandPool commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);


void createDescriptorPool(ecs_world_t * world, VkDevice device, VkDescriptorPool * descriptorPool);

#ifdef __cplusplus
}
#endif
