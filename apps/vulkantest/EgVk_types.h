#pragma once

#include "flecs.h"
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
VkInstance instance;
} EgVkInstance;

typedef struct
{
VkPhysicalDevice device;
} EgVkPhysicalDevice;

typedef struct
{
VkPresentModeKHR mode;
} EgVkPresentModeKHR;

typedef struct
{
ecs_u32_t apiVersion;
ecs_u32_t driverVersion;
ecs_u32_t vendorID;
ecs_u32_t deviceID;
} EgVkPhysicalDeviceProperties;

typedef struct
{
ecs_u32_t queueFlags;
ecs_u32_t queueCount;
ecs_u32_t timestampValidBits;
} EgVkQueueFamilyProperties;

typedef struct
{
int32_t format;
int32_t colorSpace;
} EgVkSurfaceFormatKHR;

typedef struct
{
	uint32_t specVersion;
	uint32_t implementationVersion;
} EgVkLayer;

typedef struct
{
	uint32_t specVersion;
} EgVkExtensionProperties;


extern ECS_COMPONENT_DECLARE(EgVkInstance);
extern ECS_COMPONENT_DECLARE(EgVkLayer);
extern ECS_COMPONENT_DECLARE(EgVkExtensionProperties);
extern ECS_COMPONENT_DECLARE(VkApplicationInfo);
extern ECS_COMPONENT_DECLARE(EgVkPhysicalDevice);
extern ECS_COMPONENT_DECLARE(EgVkPresentModeKHR);
extern ECS_COMPONENT_DECLARE(EgVkPhysicalDeviceProperties);
extern ECS_COMPONENT_DECLARE(EgVkQueueFamilyProperties);
extern ECS_COMPONENT_DECLARE(EgVkSurfaceFormatKHR);


extern ECS_DECLARE(EgVkExtension);
extern ECS_DECLARE(EgVkRequiredExtension);
extern ecs_entity_t VkExtensionSwapchain;

extern ECS_DECLARE(Eg_VK_QUEUE_GRAPHICS_BIT);
extern ECS_DECLARE(Eg_PhysicalDeviceSurfaceSupportKHR);


void EgVkImport(ecs_world_t *world);


#ifdef __cplusplus
}
#endif
