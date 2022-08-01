#include "EgVkLayers.h"
#include "EgVk_types.h"
#include "EgTypes.h"
#include <stdio.h>



void EgVkLayersImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgVkLayers);
	ecs_set_name_prefix(world, "EgVk");




	{
		uint32_t count;
		vkEnumerateInstanceLayerProperties(&count, NULL);
		VkLayerProperties * items = ecs_os_malloc_n(VkLayerProperties, count);
		vkEnumerateInstanceLayerProperties(&count, items);
		for (uint32_t i = 0; i < count; ++i)
		{
			char const * name = items[i].layerName;
			ecs_entity_t r = ecs_entity_init(world, &(ecs_entity_desc_t){
			.name = name
			});
			EgVkLayer l;
			l.implementationVersion = items[i].implementationVersion;
			l.specVersion = items[i].specVersion;
			ecs_set_ptr(world, r, EgVkLayer, &l);
		}
	}

}










