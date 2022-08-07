#include "render1.h"

#include "EgVk.h"
#include "EgVkInstances.h"
#include "EgWindows.h"
#include "EgVkPhysicaldevicefeatures.h"
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
		EG_TRACE(world, EgLogsError, "failed to find a suitable GPU!");
		//ecs_os_abort();
		while(1)
		{
			ecs_progress(world, 0);
		}
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

