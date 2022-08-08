#include "EgVkQueueFamilies.h"
#include "EgVk.h"
#include "EgTypes.h"
#include "EgPlatform.h"
#include "EgWindows.h"
#include <stdio.h>


ECS_COMPONENT_DECLARE(EgVkQueueFamilies);




void populate_VkQueueFamilyProperties(ecs_world_t * world, ecs_entity_t parent, VkPhysicalDevice device, VkSurfaceKHR surface)
{
	uint32_t count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, NULL);
	VkQueueFamilyProperties * items = ecs_os_malloc_n(VkQueueFamilyProperties, count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, items);
	for (uint32_t i = 0; i < count; ++i)
	{
		char name[100];
		snprintf(name, 100, "QueueFamily%i", i);
		ecs_entity_t r = ecs_entity_init(world, &(ecs_entity_desc_t)
		{
		.name = name
		});
		ecs_add_pair(world, r, EcsChildOf, parent);
		//ecs_doc_set_name(world, r, "QueueFamily");
		ecs_set_ptr(world, r, EgVkQueueFamilyProperties, items + i);
		ecs_set(world, r, EgIndex, {i});
		if (items[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			ecs_add(world, parent, Eg_VK_QUEUE_GRAPHICS_BIT);
			ecs_add(world, r, Eg_VK_QUEUE_GRAPHICS_BIT);
		}
		if (items[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
		{
			ecs_add(world, parent, Eg_VK_QUEUE_COMPUTE_BIT);
			ecs_add(world, r, Eg_VK_QUEUE_COMPUTE_BIT);
		}
		if (items[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
		{
			ecs_add(world, parent, Eg_VK_QUEUE_TRANSFER_BIT);
			ecs_add(world, r, Eg_VK_QUEUE_TRANSFER_BIT);
		}
		if (items[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
		{
			ecs_add(world, parent, Eg_VK_QUEUE_SPARSE_BINDING_BIT);
			ecs_add(world, r, Eg_VK_QUEUE_SPARSE_BINDING_BIT);
		}
		if (items[i].queueFlags & VK_QUEUE_PROTECTED_BIT)
		{
			ecs_add(world, parent, Eg_VK_QUEUE_PROTECTED_BIT);
			ecs_add(world, r, Eg_VK_QUEUE_PROTECTED_BIT);
		}
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
		if (presentSupport)
		{
			ecs_add(world, parent, Eg_PhysicalDeviceSurfaceSupportKHR);
			ecs_add(world, r, Eg_PhysicalDeviceSurfaceSupportKHR);
		}
	}
	ecs_os_free(items);
}


static void System_populate_queue_family(ecs_iter_t *it)
{
	EgVkSurfaceKHR * surface = ecs_field(it, EgVkSurfaceKHR, 1); // Parent
	EgVkPhysicalDevice * dev = ecs_field(it, EgVkPhysicalDevice, 2);
	for (int i = 0; i < it->count; i ++)
	{
		populate_VkQueueFamilyProperties(it->world, it->entities[i], dev[i].device, surface[0].surface);
	}
}



void EgVkQueueFamiliesImport(ecs_world_t *world)
{
	ECS_MODULE_DEFINE(world, EgVkQueueFamilies);
	ECS_IMPORT(world, EgPlatform);
	ECS_IMPORT(world, EgVk);
	ECS_IMPORT(world, EgTypes);
	ecs_set_name_prefix(world, "EgVk");

	ECS_OBSERVER(world, System_populate_queue_family, EcsOnSet, EgVkSurfaceKHR(parent), EgVkPhysicalDevice);

	/*
	ecs_observer_init(world, &(ecs_observer_desc_t){
	.filter.terms = {
	{ .id = ecs_id(EgVkSurfaceKHR), .src.flags = EcsUp },
	{ .id = ecs_id(EgVkPhysicalDevice) }
	},
	.events = {EcsOnSet},
	.callback = System_populate_queue_family
	});
	*/
}










