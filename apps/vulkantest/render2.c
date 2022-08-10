#include "render2.h"

#include "eg_basics.h"
#include "EgVk.h"
#include "EgLogs.h"




void render2_config(render2_context_t * ctx)
{
	{
		VkQueueFamilyProperties properties[32];
		uint32_t count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(ctx->physical, &count, NULL);
		count = EG_MIN(count, 32);
		vkGetPhysicalDeviceQueueFamilyProperties(ctx->physical, &count, properties);
		for (uint32_t i = 0; i < count; ++i)
		{
			if (properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				ctx->qf_index_graphics = i;
			}
			VkBool32 present_support = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(ctx->physical, i, ctx->surface, &present_support);
			if (present_support)
			{
				ctx->qf_index_present = i;
			}
		}
	}

	{
		VkPhysicalDeviceFeatures features = {};
		VkDeviceCreateInfo device_info = {};
		VkDeviceQueueCreateInfo queue_info[2] = {};
		features.samplerAnisotropy = VK_TRUE;
		device_info.pEnabledFeatures = &features;
		device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_info.pQueueCreateInfos = queue_info;
		float priority = 1.0f;
		queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_info[0].queueFamilyIndex = ctx->qf_index_graphics;
		queue_info[0].queueCount = 1;
		queue_info[0].pQueuePriorities = &priority;
		queue_info[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_info[1].queueFamilyIndex = ctx->qf_index_present;
		queue_info[1].queueCount = 1;
		queue_info[1].pQueuePriorities = &priority;
		if(ctx->qf_index_graphics == ctx->qf_index_present)
		{
			device_info.queueCreateInfoCount = 1;
		}
		else
		{
			device_info.queueCreateInfoCount = 2;
		}
		char const * exts[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
		char const * layers[] = {"VK_LAYER_KHRONOS_validation"};
		device_info.enabledExtensionCount = 1;
		device_info.ppEnabledExtensionNames = exts;
		device_info.enabledLayerCount = 1;
		device_info.ppEnabledLayerNames = layers;
		VkResult result = vkCreateDevice(ctx->physical, &device_info, NULL, &ctx->device);
		//EG_EVENT_STRF(world, EgVkLogVerbose, "vkCreateDevice : %i\n", result);
		if (result != VK_SUCCESS)
		{
			//EG_EVENT_STRF(world, EgVkLogError, "vkCreateDevice failed\n");
		}
		vkGetDeviceQueue(ctx->device, ctx->qf_index_graphics, 0, &ctx->qf_graphics);
		vkGetDeviceQueue(ctx->device, ctx->qf_index_present, 0, &ctx->qf_present);
	}

	{
		uint32_t count;
		VkSurfaceFormatKHR formats[32];
		vkGetPhysicalDeviceSurfaceFormatsKHR(ctx->physical, ctx->surface, &count, NULL);
		count = EG_MIN(count, 32);
		vkGetPhysicalDeviceSurfaceFormatsKHR(ctx->physical, ctx->surface, &count, formats);
		ctx->surface_format = formats[0].format;
		for (uint32_t i = 0; i < count; ++i)
		{
			VkFormat f = formats[i].format;
			VkColorSpaceKHR c = formats[i].colorSpace;
			if (f == VK_FORMAT_B8G8R8A8_SRGB && c == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				ctx->surface_format = f;
			}
		}
	}

	{
		ctx->surface_presentmode = VK_PRESENT_MODE_FIFO_KHR;
		VkPresentModeKHR modes[32];
		uint32_t count;
		vkGetPhysicalDeviceSurfacePresentModesKHR(ctx->physical, ctx->surface, &count, NULL);
		count = EG_MIN(count, 32);
		vkGetPhysicalDeviceSurfacePresentModesKHR(ctx->physical, ctx->surface, &count, modes);
		for (uint32_t i = 0; i < count; ++i)
		{
			if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				ctx->surface_presentmode = modes[i];
			}
		}
	}

	ctx->swapchain.swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	ctx->swapchain.swapchain_create_info.surface = ctx->surface;
	ctx->swapchain.swapchain_create_info.imageFormat = ctx->surface_format;
	ctx->swapchain.swapchain_create_info.imageColorSpace = ctx->surface_colorspace;
	ctx->swapchain.swapchain_create_info.imageArrayLayers = 1;
	ctx->swapchain.swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	ctx->swapchain.swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ctx->swapchain.swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	ctx->swapchain.swapchain_create_info.presentMode = ctx->surface_presentmode;
	ctx->swapchain.swapchain_create_info.clipped = VK_TRUE;
}



void render2_swapchain_create
(
ecs_world_t * world,
VkDevice device,
VkSurfaceCapabilitiesKHR * capabilities,
uint32_t width,
uint32_t height,
VkSwapchainCreateInfoKHR * swapchain_create_info,
VkSwapchainKHR * out_swapchain
)
{
	VkExtent2D extent = {width, height};
	if (capabilities->currentExtent.width != UINT32_MAX)
	{
		extent = capabilities->currentExtent;
	}

	extent.width = EG_CLAMP(extent.width, capabilities->minImageExtent.width, capabilities->maxImageExtent.width);
	extent.height = EG_CLAMP(extent.height, capabilities->minImageExtent.height, capabilities->maxImageExtent.height);

	uint32_t imageCount = capabilities->minImageCount + 1;
	if (capabilities->maxImageCount > 0 && imageCount > capabilities->maxImageCount)
	{
		imageCount = capabilities->maxImageCount;
	}

	swapchain_create_info->imageExtent = extent;
	swapchain_create_info->preTransform = capabilities->currentTransform;
	swapchain_create_info->minImageCount = imageCount;

	VkResult result = vkCreateSwapchainKHR(device, swapchain_create_info, NULL, out_swapchain);
	EG_EVENT_STRF(world, EgVkLogVerbose, "vkCreateSwapchainKHR %p : %i\n", device, result);
	if (result != VK_SUCCESS)
	{
		EG_EVENT_STRF(world, EgVkLogError, "vkCreateSwapchainKHR failed");
	}



}



