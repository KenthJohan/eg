#include "EgVkInstanceExtensions.h"
#include "EgVk.h"
#include "EgTypes.h"
#include "EgPlatform.h"
#include "EgWindows.h"
#include <stdio.h>


ECS_COMPONENT_DECLARE(EgVkInstanceExtensions);

static void Observer_Populate_Required_Extensions(ecs_iter_t *it)
{
	ecs_entity_t prev_scope = ecs_set_scope(it->world, ecs_id(EgVkInstanceExtensions));
	//TODO: Should this be called here?
	platform_populate_required_extension_names(it->world);
	ecs_set_scope(it->world, prev_scope);
}




void EgVkInstanceExtensionsImport(ecs_world_t *world)
{
	ECS_MODULE_DEFINE(world, EgVkInstanceExtensions);
	ECS_IMPORT(world, EgPlatform);
	ECS_IMPORT(world, EgVk);
	ECS_IMPORT(world, EgTypes);
	ecs_set_name_prefix(world, "EgVk");


	//ECS_OBSERVER(world, Observer_Populate_Required_Extensions, EcsOnAdd, EgWindow, EgValid);

	ecs_observer_init(world, &(ecs_observer_desc_t){
	.filter.terms = { { ecs_id(EgWindow) }},
	.events = {EcsOnAdd},
	.callback = Observer_Populate_Required_Extensions
	});

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
			.add = {EgVkExtension, EgVkInstanceExtension}
			});
			//printf("extensionName %s (%lx)\n", name, r);
			ecs_set(world, r, EgVkExtensionProperties, {items[i].specVersion});
		}
		ecs_os_free(items);
	}

	{
		ecs_entity_t r = ecs_entity_init(world, &(ecs_entity_desc_t)
		{
		.name = VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
		.add = {EgVkExtension, EgVkInstanceExtension}
		});
	}

	/*
	{
		ecs_entity_t r = ecs_entity_init(world, &(ecs_entity_desc_t)
		{
		.name = VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		.add = {EgVkExtension}
		});
	}
	*/

}










