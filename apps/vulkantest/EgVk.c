#include "EgVk.h"
#include "EgTypes.h"
#include "EgLogs.h"
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
ECS_DECLARE(EgVkDeviceExtension);
ECS_DECLARE(EgVkInstanceExtension);
ECS_DECLARE(EgVkRequiredExtension);

ECS_DECLARE(Eg_VK_QUEUE_GRAPHICS_BIT);
ECS_DECLARE(Eg_VK_QUEUE_COMPUTE_BIT);
ECS_DECLARE(Eg_VK_QUEUE_TRANSFER_BIT);
ECS_DECLARE(Eg_VK_QUEUE_SPARSE_BINDING_BIT);
ECS_DECLARE(Eg_VK_QUEUE_PROTECTED_BIT);
ECS_DECLARE(Eg_PhysicalDeviceSurfaceSupportKHR);

// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPhysicalDeviceFeatures.html
ECS_DECLARE(EgVkPhysicalDeviceFeature_robustBufferAccess);
ECS_DECLARE(EgVkPhysicalDeviceFeature_fullDrawIndexUint32);
ECS_DECLARE(EgVkPhysicalDeviceFeature_imageCubeArray);
ECS_DECLARE(EgVkPhysicalDeviceFeature_independentBlend);
ECS_DECLARE(EgVkPhysicalDeviceFeature_geometryShader);
ECS_DECLARE(EgVkPhysicalDeviceFeature_tessellationShader);
ECS_DECLARE(EgVkPhysicalDeviceFeature_sampleRateShading);
ECS_DECLARE(EgVkPhysicalDeviceFeature_dualSrcBlend);
ECS_DECLARE(EgVkPhysicalDeviceFeature_logicOp);
ECS_DECLARE(EgVkPhysicalDeviceFeature_multiDrawIndirect);
ECS_DECLARE(EgVkPhysicalDeviceFeature_drawIndirectFirstInstance);
ECS_DECLARE(EgVkPhysicalDeviceFeature_depthClamp);
ECS_DECLARE(EgVkPhysicalDeviceFeature_depthBiasClamp);
ECS_DECLARE(EgVkPhysicalDeviceFeature_fillModeNonSolid);
ECS_DECLARE(EgVkPhysicalDeviceFeature_depthBounds);
ECS_DECLARE(EgVkPhysicalDeviceFeature_wideLines);
ECS_DECLARE(EgVkPhysicalDeviceFeature_largePoints);
ECS_DECLARE(EgVkPhysicalDeviceFeature_alphaToOne);
ECS_DECLARE(EgVkPhysicalDeviceFeature_multiViewport);
ECS_DECLARE(EgVkPhysicalDeviceFeature_samplerAnisotropy);
ECS_DECLARE(EgVkPhysicalDeviceFeature_textureCompressionETC2);
ECS_DECLARE(EgVkPhysicalDeviceFeature_textureCompressionASTC_LDR);
ECS_DECLARE(EgVkPhysicalDeviceFeature_textureCompressionBC);
ECS_DECLARE(EgVkPhysicalDeviceFeature_occlusionQueryPrecise);
ECS_DECLARE(EgVkPhysicalDeviceFeature_pipelineStatisticsQuery);
ECS_DECLARE(EgVkPhysicalDeviceFeature_vertexPipelineStoresAndAtomics);
ECS_DECLARE(EgVkPhysicalDeviceFeature_fragmentStoresAndAtomics);
ECS_DECLARE(EgVkPhysicalDeviceFeature_shaderTessellationAndGeometryPointSize);
ECS_DECLARE(EgVkPhysicalDeviceFeature_shaderImageGatherExtended);
ECS_DECLARE(EgVkPhysicalDeviceFeature_shaderStorageImageExtendedFormats);
ECS_DECLARE(EgVkPhysicalDeviceFeature_shaderStorageImageMultisample);
ECS_DECLARE(EgVkPhysicalDeviceFeature_shaderStorageImageReadWithoutFormat);
ECS_DECLARE(EgVkPhysicalDeviceFeature_shaderStorageImageWriteWithoutFormat);
ECS_DECLARE(EgVkPhysicalDeviceFeature_shaderUniformBufferArrayDynamicIndexing);
ECS_DECLARE(EgVkPhysicalDeviceFeature_shaderSampledImageArrayDynamicIndexing);
ECS_DECLARE(EgVkPhysicalDeviceFeature_shaderStorageBufferArrayDynamicIndexing);
ECS_DECLARE(EgVkPhysicalDeviceFeature_shaderStorageImageArrayDynamicIndexing);
ECS_DECLARE(EgVkPhysicalDeviceFeature_shaderClipDistance);
ECS_DECLARE(EgVkPhysicalDeviceFeature_shaderCullDistance);
ECS_DECLARE(EgVkPhysicalDeviceFeature_shaderFloat64);
ECS_DECLARE(EgVkPhysicalDeviceFeature_shaderInt64);
ECS_DECLARE(EgVkPhysicalDeviceFeature_shaderInt16);
ECS_DECLARE(EgVkPhysicalDeviceFeature_shaderResourceResidency);
ECS_DECLARE(EgVkPhysicalDeviceFeature_shaderResourceMinLod);
ECS_DECLARE(EgVkPhysicalDeviceFeature_sparseBinding);
ECS_DECLARE(EgVkPhysicalDeviceFeature_sparseResidencyBuffer);
ECS_DECLARE(EgVkPhysicalDeviceFeature_sparseResidencyImage2D);
ECS_DECLARE(EgVkPhysicalDeviceFeature_sparseResidencyImage3D);
ECS_DECLARE(EgVkPhysicalDeviceFeature_sparseResidency2Samples);
ECS_DECLARE(EgVkPhysicalDeviceFeature_sparseResidency4Samples);
ECS_DECLARE(EgVkPhysicalDeviceFeature_sparseResidency8Samples);
ECS_DECLARE(EgVkPhysicalDeviceFeature_sparseResidency16Samples);
ECS_DECLARE(EgVkPhysicalDeviceFeature_sparseResidencyAliased);
ECS_DECLARE(EgVkPhysicalDeviceFeature_variableMultisampleRate);
ECS_DECLARE(EgVkPhysicalDeviceFeature_inheritedQueries);


ECS_DECLARE(Eg_VK_PRESENT_MODE_IMMEDIATE_KHR);
ECS_DECLARE(Eg_VK_PRESENT_MODE_MAILBOX_KHR);
ECS_DECLARE(Eg_VK_PRESENT_MODE_FIFO_KHR);
ECS_DECLARE(Eg_VK_PRESENT_MODE_FIFO_RELAXED_KHR);
ECS_DECLARE(Eg_VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR);
ECS_DECLARE(Eg_VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR);
ECS_DECLARE(Eg_VK_PRESENT_MODE_MAX_ENUM_KHR);


ECS_DECLARE(EgVkLogVerbose);
ECS_DECLARE(EgVkLogInfo);
ECS_DECLARE(EgVkLogWarning);
ECS_DECLARE(EgVkLogError);



void EgVkImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgVk);
	ECS_IMPORT(world, EgTypes);
	ECS_IMPORT(world, EgLogs);
	ecs_set_name_prefix(world, "EgVk");

	ECS_TAG_DEFINE(world, EgVkExtension);
	ECS_TAG_DEFINE(world, EgVkDeviceExtension);
	ECS_TAG_DEFINE(world, EgVkInstanceExtension);
	ECS_TAG_DEFINE(world, EgVkRequiredExtension);

	ECS_TAG_DEFINE(world, Eg_VK_QUEUE_GRAPHICS_BIT);
	ECS_TAG_DEFINE(world, Eg_VK_QUEUE_COMPUTE_BIT);
	ECS_TAG_DEFINE(world, Eg_VK_QUEUE_TRANSFER_BIT);
	ECS_TAG_DEFINE(world, Eg_VK_QUEUE_SPARSE_BINDING_BIT);
	ECS_TAG_DEFINE(world, Eg_VK_QUEUE_PROTECTED_BIT);
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

	ECS_TAG_DEFINE(world, EgVkLogVerbose);
	ECS_TAG_DEFINE(world, EgVkLogInfo);
	ECS_TAG_DEFINE(world, EgVkLogWarning);
	ECS_TAG_DEFINE(world, EgVkLogError);

	//typedef struct VkPhysicalDeviceFeatures
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_robustBufferAccess);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_fullDrawIndexUint32);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_imageCubeArray);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_independentBlend);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_geometryShader);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_tessellationShader);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_sampleRateShading);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_dualSrcBlend);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_logicOp);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_multiDrawIndirect);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_drawIndirectFirstInstance);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_depthClamp);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_depthBiasClamp);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_fillModeNonSolid);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_depthBounds);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_wideLines);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_largePoints);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_alphaToOne);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_multiViewport);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_samplerAnisotropy);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_textureCompressionETC2);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_textureCompressionASTC_LDR);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_textureCompressionBC);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_occlusionQueryPrecise);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_pipelineStatisticsQuery);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_vertexPipelineStoresAndAtomics);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_fragmentStoresAndAtomics);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_shaderTessellationAndGeometryPointSize);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_shaderImageGatherExtended);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_shaderStorageImageExtendedFormats);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_shaderStorageImageMultisample);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_shaderStorageImageReadWithoutFormat);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_shaderStorageImageWriteWithoutFormat);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_shaderUniformBufferArrayDynamicIndexing);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_shaderSampledImageArrayDynamicIndexing);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_shaderStorageBufferArrayDynamicIndexing);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_shaderStorageImageArrayDynamicIndexing);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_shaderClipDistance);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_shaderCullDistance);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_shaderFloat64);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_shaderInt64);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_shaderInt16);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_shaderResourceResidency);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_shaderResourceMinLod);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_sparseBinding);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_sparseResidencyBuffer);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_sparseResidencyImage2D);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_sparseResidencyImage3D);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_sparseResidency2Samples);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_sparseResidency4Samples);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_sparseResidency8Samples);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_sparseResidency16Samples);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_sparseResidencyAliased);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_variableMultisampleRate);
	ECS_TAG_DEFINE(world, EgVkPhysicalDeviceFeature_inheritedQueries);

	ECS_TAG_DEFINE(world, Eg_VK_PRESENT_MODE_IMMEDIATE_KHR);
	ECS_TAG_DEFINE(world, Eg_VK_PRESENT_MODE_MAILBOX_KHR);
	ECS_TAG_DEFINE(world, Eg_VK_PRESENT_MODE_FIFO_KHR);
	ECS_TAG_DEFINE(world, Eg_VK_PRESENT_MODE_FIFO_RELAXED_KHR);
	ECS_TAG_DEFINE(world, Eg_VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR);
	ECS_TAG_DEFINE(world, Eg_VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR);
	ECS_TAG_DEFINE(world, Eg_VK_PRESENT_MODE_MAX_ENUM_KHR);

	ecs_doc_set_color(world, EgVkLogError,   EG_LOGS_COLOR_ERROR);
	ecs_doc_set_color(world, EgVkLogVerbose, EG_LOGS_COLOR_VERBOSE);
	ecs_doc_set_color(world, EgVkLogInfo,    EG_LOGS_COLOR_INFO);
	ecs_doc_set_color(world, EgVkLogWarning, EG_LOGS_COLOR_WARNING);

	ecs_add(world, EgVkLogError,    EgEventsIdling);
	ecs_add(world, EgVkLogError,    EgEventsPrint);
	ecs_add(world, EgVkLogVerbose,  EgEventsPrint);
	ecs_add(world, EgVkLogInfo,     EgEventsPrint);
	ecs_add(world, EgVkLogWarning,  EgEventsPrint);

	ecs_struct_init(world, &(ecs_struct_desc_t){
	.entity = ecs_entity(world, {.id = ecs_id(EgVkInstance)}),
	.members = {
	{ .name = "instance", .type = ecs_id(ecs_uptr_t) },
	{ .name = "debug_messenger", .type = ecs_id(ecs_uptr_t) }
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










