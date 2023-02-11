#pragma once
#include "flecs.h"
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
	VkSwapchainCreateInfoKHR swapchain_create_info;
	VkSwapchainKHR swapchain;
	VkImageView swapchain_imageview[32];
	VkFramebuffer swapchain_framebuffer[32];
	uint32_t swapChainImageViews_count;

	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;
} render2_swapchain_t;

void swapchain_cleanup(VkDevice device, render2_swapchain_t * swapchain);
void createSwapChain(render2_swapchain_t * swapchain, ecs_world_t * world, VkPhysicalDevice physical, VkDevice device, VkSurfaceKHR surface, uint32_t width, uint32_t height, VkSampleCountFlagBits msaaSamples, VkRenderPass renderPass);


#ifdef __cplusplus
}
#endif
