#pragma once

#include "flecs.h"
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
VkInstance instance;
VkDebugUtilsMessengerEXT debug_messenger;
} EgVkInstance;

typedef struct
{
VkPhysicalDevice device;
} EgVkPhysicalDevice;

typedef struct
{
VkSurfaceKHR surface;
} EgVkSurfaceKHR;

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
extern ECS_COMPONENT_DECLARE(EgVkSurfaceKHR);

extern ECS_COMPONENT_DECLARE(EgVkExtensionProperties);
extern ECS_COMPONENT_DECLARE(VkApplicationInfo);
extern ECS_COMPONENT_DECLARE(EgVkPhysicalDevice);
extern ECS_COMPONENT_DECLARE(EgVkPresentModeKHR);
extern ECS_COMPONENT_DECLARE(EgVkPhysicalDeviceProperties);
extern ECS_COMPONENT_DECLARE(EgVkQueueFamilyProperties);
extern ECS_COMPONENT_DECLARE(EgVkSurfaceFormatKHR);


extern ECS_DECLARE(EgVkExtension);
extern ECS_DECLARE(EgVkDeviceExtension);
extern ECS_DECLARE(EgVkInstanceExtension);
extern ECS_DECLARE(EgVkRequiredExtension);

extern ECS_DECLARE(Eg_VK_QUEUE_GRAPHICS_BIT);
extern ECS_DECLARE(Eg_VK_QUEUE_COMPUTE_BIT);
extern ECS_DECLARE(Eg_VK_QUEUE_TRANSFER_BIT);
extern ECS_DECLARE(Eg_VK_QUEUE_SPARSE_BINDING_BIT);
extern ECS_DECLARE(Eg_VK_QUEUE_PROTECTED_BIT);
extern ECS_DECLARE(Eg_PhysicalDeviceSurfaceSupportKHR);


//typedef struct VkPhysicalDeviceFeatures
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_robustBufferAccess);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_fullDrawIndexUint32);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_imageCubeArray);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_independentBlend);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_geometryShader);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_tessellationShader);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_sampleRateShading);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_dualSrcBlend);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_logicOp);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_multiDrawIndirect);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_drawIndirectFirstInstance);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_depthClamp);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_depthBiasClamp);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_fillModeNonSolid);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_depthBounds);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_wideLines);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_largePoints);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_alphaToOne);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_multiViewport);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_samplerAnisotropy);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_textureCompressionETC2);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_textureCompressionASTC_LDR);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_textureCompressionBC);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_occlusionQueryPrecise);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_pipelineStatisticsQuery);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_vertexPipelineStoresAndAtomics);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_fragmentStoresAndAtomics);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_shaderTessellationAndGeometryPointSize);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_shaderImageGatherExtended);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_shaderStorageImageExtendedFormats);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_shaderStorageImageMultisample);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_shaderStorageImageReadWithoutFormat);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_shaderStorageImageWriteWithoutFormat);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_shaderUniformBufferArrayDynamicIndexing);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_shaderSampledImageArrayDynamicIndexing);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_shaderStorageBufferArrayDynamicIndexing);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_shaderStorageImageArrayDynamicIndexing);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_shaderClipDistance);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_shaderCullDistance);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_shaderFloat64);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_shaderInt64);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_shaderInt16);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_shaderResourceResidency);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_shaderResourceMinLod);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_sparseBinding);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_sparseResidencyBuffer);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_sparseResidencyImage2D);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_sparseResidencyImage3D);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_sparseResidency2Samples);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_sparseResidency4Samples);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_sparseResidency8Samples);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_sparseResidency16Samples);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_sparseResidencyAliased);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_variableMultisampleRate);
extern ECS_DECLARE(EgVkPhysicalDeviceFeature_inheritedQueries);



extern ECS_DECLARE(Eg_VK_PRESENT_MODE_IMMEDIATE_KHR);
extern ECS_DECLARE(Eg_VK_PRESENT_MODE_MAILBOX_KHR);
extern ECS_DECLARE(Eg_VK_PRESENT_MODE_FIFO_KHR);
extern ECS_DECLARE(Eg_VK_PRESENT_MODE_FIFO_RELAXED_KHR);
extern ECS_DECLARE(Eg_VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR);
extern ECS_DECLARE(Eg_VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR);
extern ECS_DECLARE(Eg_VK_PRESENT_MODE_MAX_ENUM_KHR);


extern ECS_DECLARE(EgVkLogVerbose);
extern ECS_DECLARE(EgVkLogInfo);
extern ECS_DECLARE(EgVkLogWarning);
extern ECS_DECLARE(EgVkLogError);




void EgVkImport(ecs_world_t *world);


#ifdef __cplusplus
}
#endif
