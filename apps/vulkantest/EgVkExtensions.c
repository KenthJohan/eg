#include "EgVkExtensions.h"
#include "EgVk_types.h"
#include "EgTypes.h"
#include <stdio.h>



void EgVkExtensionsImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgVkExtensions);
	ecs_set_name_prefix(world, "EgVk");

	{
		uint32_t count = 0;
		vkEnumerateInstanceExtensionProperties(NULL, &count, NULL);
		VkExtensionProperties * items = ecs_os_malloc_n(VkExtensionProperties, count);
		vkEnumerateInstanceExtensionProperties(NULL, &count, items);
		for (uint32_t i = 0; i < count; ++i)
		{
			char const * name = items[i].extensionName;
			ecs_entity_t r = ecs_entity_init(world, &(ecs_entity_desc_t){
			.name = name,
			.add = {EgVkExtension}
			});
			printf("extensionName %s (%lx)\n", name, r);
			ecs_set(world, r, EgVkExtensionProperties, {items[i].specVersion});
		}
		ecs_os_free(items);
	}

	{
		ecs_entity_t r = ecs_entity_init(world, &(ecs_entity_desc_t)
		{
		.name = VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
		.add = {EgVkExtension}
		});
	}

}










