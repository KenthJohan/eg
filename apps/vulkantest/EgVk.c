#include "EgVk.h"
#include "EgTypes.h"
#include <stdio.h>









ECS_COMPONENT_DECLARE(VkApplicationInfo);
ECS_COMPONENT_DECLARE(EgVkLayer);


//Handles:
ECS_COMPONENT_DECLARE(EgVkInstance);
ECS_COMPONENT_DECLARE(EgVkPhysicalDevice);
ECS_COMPONENT_DECLARE(EgVkSurfaceKHR);

ECS_COMPONENT_DECLARE(EgVkExtensionProperties);
ECS_COMPONENT_DECLARE(EgVkPresentModeKHR);
ECS_COMPONENT_DECLARE(EgVkPhysicalDeviceProperties);
ECS_COMPONENT_DECLARE(EgVkQueueFamilyProperties);
ECS_COMPONENT_DECLARE(EgVkSurfaceFormatKHR);


ECS_DECLARE(EgVkExtension);
ECS_DECLARE(EgVkRequiredExtension);

ECS_DECLARE(Eg_VK_QUEUE_GRAPHICS_BIT);
ECS_DECLARE(Eg_PhysicalDeviceSurfaceSupportKHR);




void EgVkImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgVk);
	ECS_IMPORT(world, EgTypes);
	ecs_set_name_prefix(world, "EgVk");

	ECS_TAG_DEFINE(world, EgVkExtension);
	ECS_TAG_DEFINE(world, EgVkRequiredExtension);
	ECS_TAG_DEFINE(world, Eg_VK_QUEUE_GRAPHICS_BIT);
	ECS_TAG_DEFINE(world, Eg_PhysicalDeviceSurfaceSupportKHR);

	//Handles
	ECS_COMPONENT_DEFINE(world, EgVkInstance);
	ECS_COMPONENT_DEFINE(world, EgVkPhysicalDevice);
	ECS_COMPONENT_DEFINE(world, EgVkSurfaceKHR);

	ECS_COMPONENT_DEFINE(world, EgVkExtensionProperties);
	ECS_COMPONENT_DEFINE(world, VkApplicationInfo);
	ECS_COMPONENT_DEFINE(world, EgVkLayer);
	ECS_COMPONENT_DEFINE(world, EgVkPresentModeKHR);
	ECS_COMPONENT_DEFINE(world, EgVkPhysicalDeviceProperties);
	ECS_COMPONENT_DEFINE(world, EgVkQueueFamilyProperties);
	ECS_COMPONENT_DEFINE(world, EgVkSurfaceFormatKHR);



	ecs_struct_init(world, &(ecs_struct_desc_t){
	.entity = ecs_entity(world, {.id = ecs_id(EgVkInstance)}),
	.members = {
	{ .name = "instance", .type = ecs_id(ecs_uptr_t) }
	}
	});

	ecs_struct_init(world, &(ecs_struct_desc_t){
	.entity = ecs_entity(world, {.id = ecs_id(EgVkExtensionProperties)}),
	.members = {
	{ .name = "specVersion", .type = ecs_id(ecs_u32_t) }
	}
	});


	/*
	typedef struct VkApplicationInfo {
		VkStructureType    sType;
		const void*        pNext;
		const char*        pApplicationName;
		uint32_t           applicationVersion;
		const char*        pEngineName;
		uint32_t           engineVersion;
		uint32_t           apiVersion;
	} VkApplicationInfo;

	*/
	ecs_struct_init(world, &(ecs_struct_desc_t){
	.entity = ecs_entity(world, {.id = ecs_id(VkApplicationInfo)}),
	.members = {
	{ .name = "sType",                    .type = ecs_id(ecs_i32_t) },
	{ .name = "pNext",                    .type = ecs_id(ecs_uptr_t) },
	{ .name = "pApplicationName",         .type = ecs_id(ecs_string_t) },
	{ .name = "applicationVersion",       .type = ecs_id(ecs_u32_t) },
	{ .name = "pEngineName",              .type = ecs_id(ecs_string_t) },
	{ .name = "engineVersion",            .type = ecs_id(ecs_u32_t) },
	{ .name = "apiVersion",               .type = ecs_id(ecs_u32_t) }
	}
	});

	ecs_struct_init(world, &(ecs_struct_desc_t){
	.entity = ecs_entity(world, {.id = ecs_id(EgVkPresentModeKHR)}),
	.members = {
	{ .name = "mode", .type = ecs_id(ecs_u32_t) }
	}
	});

	ecs_struct_init(world, &(ecs_struct_desc_t){
	.entity = ecs_entity(world, {.id = ecs_id(EgVkPhysicalDeviceProperties)}),
	.members = {
	{ .name = "apiVersion", .type = ecs_id(ecs_u32_t) },
	{ .name = "driverVersion", .type = ecs_id(ecs_u32_t) },
	{ .name = "vendorID", .type = ecs_id(ecs_u32_t) },
	{ .name = "deviceID", .type = ecs_id(ecs_u32_t) },
	}
	});

	ecs_struct_init(world, &(ecs_struct_desc_t){
	.entity = ecs_entity(world, {.id = ecs_id(EgVkQueueFamilyProperties)}),
	.members = {
	{ .name = "queueFlags", .type = ecs_id(ecs_u32_t) },
	{ .name = "queueCount", .type = ecs_id(ecs_u32_t) },
	{ .name = "timestampValidBits", .type = ecs_id(ecs_u32_t) }
	}
	});

	ecs_struct_init(world, &(ecs_struct_desc_t){
	.entity = ecs_entity(world, {.id = ecs_id(EgVkSurfaceFormatKHR)}),
	.members = {
	{ .name = "format", .type = ecs_id(ecs_i32_t) },
	{ .name = "colorSpace", .type = ecs_id(ecs_i32_t) },
	}
	});

	ecs_struct_init(world, &(ecs_struct_desc_t){
	.entity = ecs_entity(world, {.id = ecs_id(EgVkLayer)}),
	.members = {
	{ .name = "specVersion", .type = ecs_id(ecs_u32_t) },
	{ .name = "implementationVersion", .type = ecs_id(ecs_u32_t) },
	}
	});



	/*
	{
		VkExtensionSwapchain = ecs_entity_init(world, &(ecs_entity_desc_t)
		{
		.name = VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		.add = {EgVkExtension}
		});

		printf("VK_KHR_SWAPCHAIN_EXTENSION_NAME %s (%lx)\n", VK_KHR_SWAPCHAIN_EXTENSION_NAME, VkExtensionSwapchain);
	}
	*/
}










