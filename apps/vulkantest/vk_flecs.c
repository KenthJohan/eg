#include "vk_flecs.h"
#include <stdio.h>


ECS_COMPONENT_DECLARE(VkPhysicalDevice);

ECS_STRUCT(EgVkPhysicalDeviceProperties, {
uint32_t apiVersion;
uint32_t driverVersion;
uint32_t vendorID;
uint32_t deviceID;
});

ECS_STRUCT(EgVkQueueFamilyProperties, {
uint32_t        queueFlags;
uint32_t        queueCount;
uint32_t        timestampValidBits;
});


ECS_DECLARE(Eg_VK_QUEUE_GRAPHICS_BIT);
ECS_DECLARE(Eg_PhysicalDeviceSurfaceSupportKHR);


void ecsvk_init(ecs_world_t * world)
{
	ecs_singleton_set(world, EcsRest, {0});

	ECS_TAG_DEFINE(world, Eg_VK_QUEUE_GRAPHICS_BIT);
	ECS_TAG_DEFINE(world, Eg_PhysicalDeviceSurfaceSupportKHR);
	ECS_META_COMPONENT(world, EgVkPhysicalDeviceProperties);
	ECS_META_COMPONENT(world, EgVkQueueFamilyProperties);


}

void findQueueFamilies(ecs_world_t * world, ecs_entity_t e, VkPhysicalDevice device, VkSurfaceKHR surface)
{
	uint32_t count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, NULL);

	VkQueueFamilyProperties * q = ecs_os_malloc_n(VkQueueFamilyProperties, count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, q);

	for (uint32_t i = 0; i < count; ++i)
	{
		ecs_entity_t r = ecs_new(world, 0);
		ecs_add_pair(world, r, EcsChildOf, e);
		ecs_doc_set_name(world, r, "VkQueueFamily");

		ecs_set_ptr(world, r, EgVkQueueFamilyProperties, q+i);

		if (q[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			ecs_add(world, r, Eg_VK_QUEUE_GRAPHICS_BIT);
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		if (presentSupport)
		{
			ecs_add(world, r, Eg_PhysicalDeviceSurfaceSupportKHR);
		}
	}
}






void ecsvk_phyiscial(ecs_world_t * world, VkInstance instance, VkSurfaceKHR surface)
{
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
		/*
		ecs_entity_t r = ecs_entity_init(world, &(ecs_entity_desc_t)
		{
		//.name = props.deviceName
		.name = name
		});
		*/

		ecs_entity_t r = ecs_new_entity(world, name);
		ecs_set_ptr(world, r, EgVkPhysicalDeviceProperties, &props);
		ecs_doc_set_name(world, r, props.deviceName);

		findQueueFamilies(world, r, devices[i], surface);

	}

}
