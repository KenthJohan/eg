#include "swapchain.h"
#include "render1.h"

#include "eg_basics.h"
#include "EgVk.h"
#include "EgLogs.h"




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




void createSwapChain
(
render2_swapchain_t * swapchain,
ecs_world_t * world,
VkPhysicalDevice physical,
VkDevice device,
VkSurfaceKHR surface,
uint32_t width, uint32_t height,
VkSampleCountFlagBits msaaSamples,
VkRenderPass renderPass
)
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



