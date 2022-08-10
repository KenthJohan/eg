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

typedef struct
{
	VkPhysicalDevice physical;
	VkDevice device;
	VkSurfaceKHR surface;
	VkFormat surface_format;
	VkColorSpaceKHR surface_colorspace;
	VkPresentModeKHR surface_presentmode;
	uint32_t width;
	uint32_t height;
	uint32_t qf_index_present;
	uint32_t qf_index_graphics;
	VkQueue qf_present;
	VkQueue qf_graphics;

	render2_swapchain_t swapchain;

} render2_context_t;


void render2_config(render2_context_t * ctx);



void render2_swapchain_create
(
ecs_world_t * world,
VkDevice device,
VkSurfaceCapabilitiesKHR * capabilities,
uint32_t width,
uint32_t height,
VkSwapchainCreateInfoKHR * swapchain_create_info,
VkSwapchainKHR * out_swapchain
);



void createSwapChain(render2_swapchain_t * swapchain, ecs_world_t * world, VkPhysicalDevice physical, VkDevice device, VkSurfaceKHR surface, uint32_t width, uint32_t height, VkSampleCountFlagBits msaaSamples, VkRenderPass renderPass);


#ifdef __cplusplus
}
#endif
