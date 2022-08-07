#include "create_image.h"

#include "eg_basics.h"
#include "EgLogs.h"
#include "render1.h"
#include "stb_image.h"
#include <math.h>


#define TEXTURE_PATH "viking_room.png"

void createTextureImage
(
ecs_world_t * world,
VkPhysicalDevice physicalDevice, VkDevice device,
VkCommandPool commandPool, VkQueue graphicsQueue,
VkImage * textureImage, uint32_t * mipLevels, VkDeviceMemory * textureImageMemory
)
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(TEXTURE_PATH, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;
	(*mipLevels) = floor(log2(EG_MAX(texWidth, texHeight))) + 1;

	if (!pixels)
	{
		EG_EVENT_STRF(world, EgLogsError, "failed to load texture image!");
	}

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(world, physicalDevice, device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, imageSize);
	vkUnmapMemory(device, stagingBufferMemory);

	stbi_image_free(pixels);

	createImage
	(
	world,
	physicalDevice, device,
	texWidth, texHeight, *mipLevels,
	VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
	VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	textureImage, textureImageMemory
	);

	transitionImageLayout(world, device, commandPool, graphicsQueue, *textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, *mipLevels);
	copyBufferToImage(device, commandPool, graphicsQueue, stagingBuffer, *textureImage, texWidth, texHeight);
	//transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps

	vkDestroyBuffer(device, stagingBuffer, NULL);
	vkFreeMemory(device, stagingBufferMemory, NULL);

	generateMipmaps(world, physicalDevice, device, commandPool, graphicsQueue, *textureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, *mipLevels);
}
