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
VkPhysicalDevice phys, VkDevice dev,
VkCommandPool cmdpool, VkQueue queue,
VkImage * image, uint32_t * mipLevels, VkDeviceMemory * textureImageMemory
)
{
	int w;
	int h;
	int c;
	char const * path = TEXTURE_PATH "a";
	stbi_uc* pixels = stbi_load(path, &w, &h, &c, STBI_rgb_alpha);
	VkDeviceSize size = w * h * 4;
	(*mipLevels) = floor(log2(EG_MAX(w, h))) + 1;

	if (!pixels)
	{
		EG_EVENT_STRF(world, EgLogsError, "Failed to load texture image (%s)\n", path);
	}

	VkBuffer buffer;
	VkDeviceMemory memory;
	{
		VkMemoryPropertyFlags prop = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		createBuffer(world,phys, dev, size, usage, prop, &buffer, &memory);
	}


	void* data;
	vkMapMemory(dev, memory, 0, size, 0, &data);
	memcpy(data, pixels, size);
	vkUnmapMemory(dev, memory);

	stbi_image_free(pixels);

	{
		VkMemoryPropertyFlags prop = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		createImage
		(world, phys, dev, w, h, *mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
		usage,
		prop,
		image, textureImageMemory
		);
	}


	transitionImageLayout(world, dev, cmdpool, queue, *image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, *mipLevels);
	copyBufferToImage(dev, cmdpool, queue, buffer, *image, w, h);
	//transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps

	vkDestroyBuffer(dev, buffer, NULL);
	vkFreeMemory(dev, memory, NULL);

	generateMipmaps(world, phys, dev, cmdpool, queue, *image, VK_FORMAT_R8G8B8A8_SRGB, w, h, *mipLevels);
}
