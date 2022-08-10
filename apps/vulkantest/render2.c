#include "render2.h"
#include "render1.h"

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

















void swapchain_cleanup(VkDevice device, render2_swapchain_t * swapchain)
{
	vkDestroyImageView(device, swapchain->depthImageView, NULL);
	vkDestroyImage(device, swapchain->depthImage, NULL);
	vkFreeMemory(device, swapchain->depthImageMemory, NULL);

	vkDestroyImageView(device, swapchain->colorImageView, NULL);
	vkDestroyImage(device, swapchain->colorImage, NULL);
	vkFreeMemory(device, swapchain->colorImageMemory, NULL);

	for(uint32_t i = 0; i < swapchain->swapChainImageViews_count; ++i)
	{
		vkDestroyFramebuffer(device, swapchain->swapchain_framebuffer[i], NULL);
		vkDestroyImageView(device, swapchain->swapchain_imageview[i], NULL);
	}

	vkDestroySwapchainKHR(device, swapchain->swapchain, NULL);
}






void createSwapChain(render2_swapchain_t * swapchain, ecs_world_t * world, VkPhysicalDevice physical, VkDevice device, VkSurfaceKHR surface, uint32_t width, uint32_t height, VkSampleCountFlagBits msaaSamples, VkRenderPass renderPass)
{
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical, surface, &capabilities);
	render2_swapchain_create(world, device, &capabilities, width, height, &(swapchain->swapchain_create_info), &(swapchain->swapchain));

	{
		VkFormat colorFormat = swapchain->swapchain_create_info.imageFormat;
		uint32_t width = swapchain->swapchain_create_info.imageExtent.width;
		uint32_t height = swapchain->swapchain_create_info.imageExtent.height;
		createImage
		(
		world,
		physical, device,
		width, height, 1, msaaSamples, colorFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&swapchain->colorImage, &(swapchain->colorImageMemory)
		);
		swapchain->colorImageView = createImageView(world, device, swapchain->colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}

	{
		VkFormat depthFormat = findDepthFormat(physical);
		uint32_t width = swapchain->swapchain_create_info.imageExtent.width;
		uint32_t height = swapchain->swapchain_create_info.imageExtent.height;

		createImage
		(
		world,
		physical, device,
		width, height, 1, msaaSamples, depthFormat,
		VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&swapchain->depthImage, &swapchain->depthImageMemory
		);
		swapchain->depthImageView = createImageView(world, device, swapchain->depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
	}

	{
		uint32_t count;
		VkImage images[32];
		vkGetSwapchainImagesKHR(device, swapchain->swapchain, &count, NULL);
		count = EG_MIN(count, 32);
		vkGetSwapchainImagesKHR(device, swapchain->swapchain, &count, images);
		swapchain->swapChainImageViews_count = count;
		for (uint32_t i = 0; i < count; i++)
		{
			//vkCreateImageView
			swapchain->swapchain_imageview[i] = createImageView(world, device, images[i], swapchain->swapchain_create_info.imageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		}
	}

	{
		for (size_t i = 0; i < swapchain->swapChainImageViews_count; i++)
		{
			VkImageView attachments[3] = {swapchain->colorImageView, swapchain->depthImageView, swapchain->swapchain_imageview[i]};
			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = 3;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = swapchain->swapchain_create_info.imageExtent.width;
			framebufferInfo.height = swapchain->swapchain_create_info.imageExtent.height;
			framebufferInfo.layers = 1;
			VkResult result = vkCreateFramebuffer(device, &framebufferInfo, NULL, &swapchain->swapchain_framebuffer[i]);
			if (result != VK_SUCCESS)
			{
				EG_EVENT_STRF(world, EgVkLogError, "vkCreateFramebuffer failed");
			}
		}
	}

}



