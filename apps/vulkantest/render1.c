#include "render1.h"

#include "EgVk.h"
#include "EgVkInstances.h"
#include "EgWindows.h"
#include "eg_util.h"

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

