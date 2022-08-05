#include "EgVkDeviceExtensions.h"
#include "EgVk.h"
#include "EgTypes.h"
#include "platform.h"
#include "EgWindows.h"
#include <stdio.h>


ECS_COMPONENT_DECLARE(EgVkDeviceExtensions);


void populate_VkExtensionProperties(ecs_world_t * world, ecs_entity_t parent, VkPhysicalDevice device)
{
	uint32_t count = 0;
	vkEnumerateDeviceExtensionProperties(device, NULL, &count, NULL);
	VkExtensionProperties * items = ecs_os_malloc_n(VkExtensionProperties, count);
	vkEnumerateDeviceExtensionProperties(device, NULL, &count, items);
	for (uint32_t i = 0; i < count; ++i)
	{
		char const * name = items[i].extensionName;
		ecs_entity_t r = ecs_entity_init(world, &(ecs_entity_desc_t){
		.name = name,
		.add = {EgVkExtension, EgVkDeviceExtension}
		});
		//printf("extensionName %s (%lx)\n", name, r);
		ecs_set(world, r, EgVkExtensionProperties, {items[i].specVersion});
		ecs_add_id(world, parent, r);
	}
	ecs_os_free(items);
}



static void System_populate_VkExtensionProperties(ecs_iter_t *it)
{
	ecs_entity_t scope = ecs_set_scope(it->world, ecs_id(EgVkDeviceExtensions));
	EgVkPhysicalDevice * d = ecs_field(it, EgVkPhysicalDevice, 1);
	for (int i = 0; i < it->count; i ++)
	{
		populate_VkExtensionProperties(it->world, it->entities[i], d[i].device);
	}
	ecs_set_scope(it->world, scope);
}



void EgVkDeviceExtensionsImport(ecs_world_t *world)
{
	ECS_MODULE_DEFINE(world, EgVkDeviceExtensions);
	ECS_IMPORT(world, EgPlatform);
	ECS_IMPORT(world, EgVk);
	ECS_IMPORT(world, EgTypes);
	ecs_set_name_prefix(world, "EgVk");

	ECS_OBSERVER(world, System_populate_VkExtensionProperties, EcsOnSet, EgVkPhysicalDevice);
}










