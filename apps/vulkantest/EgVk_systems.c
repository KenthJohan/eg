#include "EgVk_systems.h"
#include "EgVk.h"
#include "EgVkPhysicaldevicefeatures.h"
#include "EgTypes.h"
#include "platform.h"
#include "eg_util.h"
#include <stdio.h>



void populate_VkSurfaceFormatKHR(ecs_world_t * world, ecs_entity_t parent, VkPhysicalDevice device, VkSurfaceKHR surface)
{
	uint32_t count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, NULL);
	VkSurfaceFormatKHR * items = ecs_os_malloc_n(VkSurfaceFormatKHR, count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, items);
	for (uint32_t i = 0; i < count; ++i)
	{
		ecs_entity_t r = ecs_new(world, 0);
		ecs_add_pair(world, r, EcsChildOf, parent);
		ecs_doc_set_name(world, r, "SurfaceFormatKHR");
		ecs_set_ptr(world, r, EgVkSurfaceFormatKHR, items + i);
	}
	ecs_os_free(items);
}


void populate_VkPresentModeKHR(ecs_world_t * world, ecs_entity_t parent, VkPhysicalDevice device, VkSurfaceKHR surface)
{
	uint32_t count;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, NULL);
	VkPresentModeKHR * items = ecs_os_malloc_n(VkPresentModeKHR, count);
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, items);
	for (uint32_t i = 0; i < count; ++i)
	{
		ecs_entity_t r = ecs_new(world, 0);
		ecs_add_pair(world, r, EcsChildOf, parent);
		ecs_doc_set_name(world, r, "PresentModeKHR");
		ecs_set(world, r, EgVkPresentModeKHR, {items[i]});
	}
	ecs_os_free(items);
}


















void populate_VkPhysicalDevice(ecs_world_t * world, ecs_entity_t parent, VkSurfaceKHR surface)
{
	VkInstance instance = ecs_get(world, parent, EgVkInstance)->instance;
	uint32_t count = 0;
	vkEnumeratePhysicalDevices(instance, &count, NULL);
	VkPhysicalDevice * devices = ecs_os_malloc_n(VkPhysicalDevice, count);
	vkEnumeratePhysicalDevices(instance, &count, devices);
	for (uint32_t i = 0; i < count; ++i)
	{
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(devices[i], &props);
		char name[100];
		snprintf(name, 100, "gpu%i", props.deviceID);
		ecs_entity_t r = ecs_new_entity(world, name);
		ecs_add_pair(world, r, EcsChildOf, parent);

		ecs_entity_t scope = ecs_set_scope(world, r);
		ecs_set(world, r, EgVkPhysicalDevice, {devices[i]});
		ecs_set_ptr(world, r, EgVkPhysicalDeviceProperties, &props);
		ecs_doc_set_name(world, r, props.deviceName);



		populate_VkQueueFamilyProperties(world, r, devices[i], surface);
		populate_VkSurfaceFormatKHR(world, r, devices[i], surface);
		populate_VkPresentModeKHR(world, r, devices[i], surface);



		ecs_set_scope(world, scope);
	}
	ecs_os_free(devices);
}







static void System_Init(ecs_iter_t *it)
{
	for (int i = 0; i < it->count; i ++)
	{

	}
}



void EgVkSystemsImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgVkSystems);
	ECS_IMPORT(world, EgVk);
	ECS_IMPORT(world, EgVkPhysicaldevicefeatures);
	ECS_IMPORT(world, EgTypes);
	ecs_set_name_prefix(world, "EgVk");



}















