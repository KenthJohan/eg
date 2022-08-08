#include "render1.h"

#include "EgVk.h"
#include "EgVkInstances.h"
#include "EgWindows.h"
#include "EgLogs.h"
#include "eg_util.h"



VkSampleCountFlagBits render1_get_max_usable_sample_count(VkPhysicalDevice physicalDevice)
{
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
	VkSampleCountFlags color = physicalDeviceProperties.limits.framebufferColorSampleCounts;
	VkSampleCountFlags depth = physicalDeviceProperties.limits.framebufferDepthSampleCounts;
	VkSampleCountFlags counts = color & depth;
	if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
	if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
	if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
	if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
	if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
	if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }
	return VK_SAMPLE_COUNT_1_BIT;
}


VkPhysicalDevice render1_pick_physical_device(ecs_world_t * world)
{
	//populate_VkPhysicalDevice(world, entity_instance, surface);
	VkPhysicalDevice device = NULL;
	ecs_filter_desc_t d =
	{
	.terms = {
	{ ecs_id(EgVkPhysicalDevice) },
	{ ecs_id(Eg_PhysicalDeviceSurfaceSupportKHR) },
	//{ VkExtensionSwapchain },
	{ ecs_id(Eg_VK_QUEUE_GRAPHICS_BIT) },
	{ ecs_id(EgVkPhysicalDeviceFeature_samplerAnisotropy) }
	}
	};
	ecs_filter_t *f = ecs_filter_init(world, &d);
	EgVkPhysicalDevice const * p = (EgVkPhysicalDevice const *) eg_get_first_from_filter(world, f);
	if (p)
	{
		device = p->device;
	}
	ecs_filter_fini(f);
	/*
	ecs_query_desc_t desc = {};
	desc.filter.expr =
	"EgVkPhysicalDevice, "
	VK_KHR_SWAPCHAIN_EXTENSION_NAME ", "
	"eg.vk.PhysicalDeviceSurfaceSupportKHR, "
	"eg.vk.VK_QUEUE_GRAPHICS_BIT, "
	"eg.vk.samplerAnisotropy";
	ecs_query_t * q = ecs_query_init(world, &desc);
	ecs_iter_t it = ecs_query_iter(world, q);
	while (ecs_query_next(&it))
	{
		printf("iter count: %i\n", it.count);
		EgVkPhysicalDevice * p = ecs_term(&it, EgVkPhysicalDevice, 1);
		if(it.count > 0){physicalDevice = p->device;}
	}
	ecs_query_fini(q);
	*/

	if (device == NULL)
	{
		EG_EVENT_STRF(world, EgLogsError, "failed to find a suitable GPU!");
	}

	return device;
}




ecs_entity_t render1_init(ecs_world_t * world)
{
	ecs_entity_t e = ecs_new(world, 0);
	ecs_add_pair(world, e, EcsChildOf, ecs_id(EgVkInstances));
	ecs_add(world, e, EgWindow);

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	{
		ecs_entity_t r = ecs_lookup_fullpath(world, "eg.vk.layers.VK_LAYER_KHRONOS_validation");
		ecs_add_id(world, e, r);
	}

	{
		ecs_entity_t r = ecs_lookup_fullpath(world, "eg.vk.instance.extensions."VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		ecs_add_id(world, e, r);
	}



	{
		//ecs_entity_t r = ecs_lookup_fullpath(world, "eg.vk.extensions."VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		//ecs_add_id(world, windowe, r);
	}

	{
		ecs_filter_t f = ECS_FILTER_INIT;
		ecs_filter_init(world, &(ecs_filter_desc_t){
		.storage = &f,
		.terms = {
		{ ecs_id(EgVkRequiredExtension) },
		}
		});
		eg_add_all_from_filter(world, e, &f);
		ecs_filter_fini(&f);
	}

	//while(1) ecs_progress(world, 0);

	ecs_set_ptr(world, e, VkApplicationInfo, &appInfo);

	return e;
}




VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool)
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void endSingleTimeCommands(VkDevice device, VkCommandBuffer commandBuffer, VkQueue graphicsQueue, VkCommandPool commandPool)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);

	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}








void copyBufferToImage(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = (VkOffset3D){0, 0, 0};
	region.imageExtent = (VkExtent3D){width,height,1};

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	endSingleTimeCommands(device, commandBuffer, graphicsQueue, commandPool);
}


void generateMipmaps(ecs_world_t * world, VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
{
	// Check if image format supports linear blitting
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
	{
		EG_EVENT_STRF(world, EgLogsError, "texture image format does not support linear blitting!");

	}

	VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = texWidth;
	int32_t mipHeight = texHeight;

	for (uint32_t i = 1; i < mipLevels; i++)
	{
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
		0, NULL,
		0, NULL,
		1, &barrier);

		VkImageBlit blit = {};
		blit.srcOffsets[0] = (VkOffset3D){0, 0, 0};
		blit.srcOffsets[1] = (VkOffset3D){mipWidth, mipHeight, 1};
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = (VkOffset3D){0, 0, 0};
		blit.dstOffsets[1] = (VkOffset3D){ mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(commandBuffer,
		image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, &blit,
		VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, NULL,
		0, NULL,
		1, &barrier);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer,
	VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
	0, NULL,
	0, NULL,
	1, &barrier);

	endSingleTimeCommands(device, commandBuffer, graphicsQueue, commandPool);
}





void transitionImageLayout(ecs_world_t * world, VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else {
		EG_EVENT_STRF(world, EgLogsError, "unsupported layout transition!");
	}

	vkCmdPipelineBarrier(
	commandBuffer,
	sourceStage, destinationStage,
	0,
	0, NULL,
	0, NULL,
	1, &barrier
	);

	endSingleTimeCommands(device, commandBuffer, graphicsQueue, commandPool);
}



uint32_t findMemoryType(ecs_world_t * world, VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}
	EG_EVENT_STRF(world, EgLogsError, "failed to find suitable memory type!");
}





void createBuffer(ecs_world_t * world, VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer * buffer, VkDeviceMemory * bufferMemory)
{
	{
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VkResult result = vkCreateBuffer(device, &bufferInfo, NULL, buffer);
		if (result != VK_SUCCESS)
		{
			EG_EVENT_STRF(world, EgLogsError, "vkCreateBuffer failed");
		}
	}
	{
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, (*buffer), &memRequirements);
		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(world, physicalDevice, memRequirements.memoryTypeBits, properties);
		VkResult result = vkAllocateMemory(device, &allocInfo, NULL, bufferMemory);
		if (result != VK_SUCCESS)
		{
			EG_EVENT_STRF(world, EgVkLogError, "vkAllocateMemory failed\n");
		}
		vkBindBufferMemory(device, (*buffer), (*bufferMemory), 0);
	}
}


void createImage
(
ecs_world_t * world,
VkPhysicalDevice physicalDevice, VkDevice device,
uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples,
VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
VkMemoryPropertyFlags properties,
VkImage * image, VkDeviceMemory * imageMemory
)
{
	{
		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = numSamples;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VkResult result = vkCreateImage(device, &imageInfo, NULL, image);
		EG_EVENT_STRF(world, EgVkLogVerbose, "vkCreateImage %p imageInfo NULL %p : %i\n", device, image, result);
		if (result != VK_SUCCESS)
		{
			EG_EVENT_STRF(world, EgVkLogError, "vkCreateImage failed\n");
		}
	}

	{
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, (*image), &memRequirements);
		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(world, physicalDevice, memRequirements.memoryTypeBits, properties);
		VkResult result = vkAllocateMemory(device, &allocInfo, NULL, imageMemory);
		if (result != VK_SUCCESS)
		{
			EG_EVENT_STRF(world, EgLogsError, "vkAllocateMemory failed");
		}
		vkBindImageMemory(device, (*image), (*imageMemory), 0);
	}
}




void createDescriptorSetLayout(ecs_world_t * world, VkDevice device, VkDescriptorSetLayout * descriptorSetLayout)
{
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.pImmutableSamplers = NULL;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = NULL;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding bindings[2] = {uboLayoutBinding, samplerLayoutBinding};
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 2;
	layoutInfo.pBindings = bindings;

	VkResult result = vkCreateDescriptorSetLayout(device, &layoutInfo, NULL, descriptorSetLayout);
	EG_EVENT_STRF(world, EgVkLogVerbose, "vkCreateDescriptorSetLayout %p layoutInfo NULL %p : %i\n", device, descriptorSetLayout, result);
	if (result != VK_SUCCESS)
	{
		EG_EVENT_STRF(world, EgVkLogError, "vkCreateDescriptorSetLayout failed");
	}
}




void copyBuffer(VkDevice device, VkQueue graphicsQueue, VkCommandPool commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);
	VkBufferCopy copyRegion = {};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
	endSingleTimeCommands(device, commandBuffer, graphicsQueue, commandPool);
}



void createDescriptorPool(ecs_world_t * world, VkDevice device, VkDescriptorPool * descriptorPool)
{
	VkDescriptorPoolSize poolSizes[2] = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = MAX_FRAMES_IN_FLIGHT;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = MAX_FRAMES_IN_FLIGHT;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 2;
	poolInfo.pPoolSizes = poolSizes;
	poolInfo.maxSets = MAX_FRAMES_IN_FLIGHT;

	VkResult result = vkCreateDescriptorPool(device, &poolInfo, NULL, descriptorPool);
	EG_EVENT_STRF(world, EgVkLogVerbose, "vkCreateDescriptorPool %p poolInfo NULL %p\n", device, descriptorPool);
	if (result != VK_SUCCESS)
	{
		EG_EVENT_STRF(world, EgVkLogError, "vkCreateDescriptorPool failed");
	}
}



















