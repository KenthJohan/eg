#pragma once
#include "flecs.h"
#include <vulkan/vulkan.h>
#include "swapchain.h"

#ifdef __cplusplus
extern "C" {
#endif

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



#ifdef __cplusplus
}
#endif
