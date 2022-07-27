#include "EgVk.h"
#include "EgTypes.h"
#include <stdio.h>









ECS_COMPONENT_DECLARE(VkApplicationInfo);
ECS_COMPONENT_DECLARE(EgVkLayer);



ECS_COMPONENT_DECLARE(EgVkInstance);
ECS_COMPONENT_DECLARE(EgVkExtensionProperties);
ECS_COMPONENT_DECLARE(EgVkPhysicalDevice);
ECS_COMPONENT_DECLARE(EgVkPresentModeKHR);
ECS_COMPONENT_DECLARE(EgVkPhysicalDeviceProperties);
ECS_COMPONENT_DECLARE(EgVkQueueFamilyProperties);
ECS_COMPONENT_DECLARE(EgVkSurfaceFormatKHR);


ECS_DECLARE(EgVkExtension);
ECS_DECLARE(EgVkRequiredExtension);
ecs_entity_t VkExtensionSwapchain;

ECS_DECLARE(Eg_VK_QUEUE_GRAPHICS_BIT);
ECS_DECLARE(Eg_PhysicalDeviceSurfaceSupportKHR);

// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPhysicalDeviceFeatures.html
ECS_DECLARE(Eg_robustBufferAccess);
ECS_DECLARE(Eg_fullDrawIndexUint32);
ECS_DECLARE(Eg_imageCubeArray);
ECS_DECLARE(Eg_independentBlend);
ECS_DECLARE(Eg_geometryShader);
ECS_DECLARE(Eg_tessellationShader);
ECS_DECLARE(Eg_sampleRateShading);
ECS_DECLARE(Eg_dualSrcBlend);
ECS_DECLARE(Eg_logicOp);
ECS_DECLARE(Eg_multiDrawIndirect);
ECS_DECLARE(Eg_drawIndirectFirstInstance);
ECS_DECLARE(Eg_depthClamp);
ECS_DECLARE(Eg_depthBiasClamp);
ECS_DECLARE(Eg_fillModeNonSolid);
ECS_DECLARE(Eg_depthBounds);
ECS_DECLARE(Eg_wideLines);
ECS_DECLARE(Eg_largePoints);
ECS_DECLARE(Eg_alphaToOne);
ECS_DECLARE(Eg_multiViewport);
ECS_DECLARE(Eg_samplerAnisotropy);
ECS_DECLARE(Eg_textureCompressionETC2);
ECS_DECLARE(Eg_textureCompressionASTC_LDR);
ECS_DECLARE(Eg_textureCompressionBC);
ECS_DECLARE(Eg_occlusionQueryPrecise);
ECS_DECLARE(Eg_pipelineStatisticsQuery);
ECS_DECLARE(Eg_vertexPipelineStoresAndAtomics);
ECS_DECLARE(Eg_fragmentStoresAndAtomics);
ECS_DECLARE(Eg_shaderTessellationAndGeometryPointSize);
ECS_DECLARE(Eg_shaderImageGatherExtended);
ECS_DECLARE(Eg_shaderStorageImageExtendedFormats);
ECS_DECLARE(Eg_shaderStorageImageMultisample);
ECS_DECLARE(Eg_shaderStorageImageReadWithoutFormat);
ECS_DECLARE(Eg_shaderStorageImageWriteWithoutFormat);
ECS_DECLARE(Eg_shaderUniformBufferArrayDynamicIndexing);
ECS_DECLARE(Eg_shaderSampledImageArrayDynamicIndexing);
ECS_DECLARE(Eg_shaderStorageBufferArrayDynamicIndexing);
ECS_DECLARE(Eg_shaderStorageImageArrayDynamicIndexing);
ECS_DECLARE(Eg_shaderClipDistance);
ECS_DECLARE(Eg_shaderCullDistance);
ECS_DECLARE(Eg_shaderFloat64);
ECS_DECLARE(Eg_shaderInt64);
ECS_DECLARE(Eg_shaderInt16);
ECS_DECLARE(Eg_shaderResourceResidency);
ECS_DECLARE(Eg_shaderResourceMinLod);
ECS_DECLARE(Eg_sparseBinding);
ECS_DECLARE(Eg_sparseResidencyBuffer);
ECS_DECLARE(Eg_sparseResidencyImage2D);
ECS_DECLARE(Eg_sparseResidencyImage3D);
ECS_DECLARE(Eg_sparseResidency2Samples);
ECS_DECLARE(Eg_sparseResidency4Samples);
ECS_DECLARE(Eg_sparseResidency8Samples);
ECS_DECLARE(Eg_sparseResidency16Samples);
ECS_DECLARE(Eg_sparseResidencyAliased);
ECS_DECLARE(Eg_variableMultisampleRate);
ECS_DECLARE(Eg_inheritedQueries);




void EgVkImport(ecs_world_t *world)
{
	ecs_singleton_set(world, EcsRest, {0});

	ecs_set_name_prefix(world, "Eg");
	ECS_MODULE(world, EgVk);
	ECS_IMPORT(world, EgTypes);

	ECS_TAG_DEFINE(world, EgVkExtension);
	ECS_TAG_DEFINE(world, EgVkRequiredExtension);
	ECS_TAG_DEFINE(world, Eg_VK_QUEUE_GRAPHICS_BIT);
	ECS_TAG_DEFINE(world, Eg_PhysicalDeviceSurfaceSupportKHR);

	//typedef struct VkPhysicalDeviceFeatures
	ECS_TAG_DEFINE(world, Eg_robustBufferAccess);
	ECS_TAG_DEFINE(world, Eg_fullDrawIndexUint32);
	ECS_TAG_DEFINE(world, Eg_imageCubeArray);
	ECS_TAG_DEFINE(world, Eg_independentBlend);
	ECS_TAG_DEFINE(world, Eg_geometryShader);
	ECS_TAG_DEFINE(world, Eg_tessellationShader);
	ECS_TAG_DEFINE(world, Eg_sampleRateShading);
	ECS_TAG_DEFINE(world, Eg_dualSrcBlend);
	ECS_TAG_DEFINE(world, Eg_logicOp);
	ECS_TAG_DEFINE(world, Eg_multiDrawIndirect);
	ECS_TAG_DEFINE(world, Eg_drawIndirectFirstInstance);
	ECS_TAG_DEFINE(world, Eg_depthClamp);
	ECS_TAG_DEFINE(world, Eg_depthBiasClamp);
	ECS_TAG_DEFINE(world, Eg_fillModeNonSolid);
	ECS_TAG_DEFINE(world, Eg_depthBounds);
	ECS_TAG_DEFINE(world, Eg_wideLines);
	ECS_TAG_DEFINE(world, Eg_largePoints);
	ECS_TAG_DEFINE(world, Eg_alphaToOne);
	ECS_TAG_DEFINE(world, Eg_multiViewport);
	ECS_TAG_DEFINE(world, Eg_samplerAnisotropy);
	ECS_TAG_DEFINE(world, Eg_textureCompressionETC2);
	ECS_TAG_DEFINE(world, Eg_textureCompressionASTC_LDR);
	ECS_TAG_DEFINE(world, Eg_textureCompressionBC);
	ECS_TAG_DEFINE(world, Eg_occlusionQueryPrecise);
	ECS_TAG_DEFINE(world, Eg_pipelineStatisticsQuery);
	ECS_TAG_DEFINE(world, Eg_vertexPipelineStoresAndAtomics);
	ECS_TAG_DEFINE(world, Eg_fragmentStoresAndAtomics);
	ECS_TAG_DEFINE(world, Eg_shaderTessellationAndGeometryPointSize);
	ECS_TAG_DEFINE(world, Eg_shaderImageGatherExtended);
	ECS_TAG_DEFINE(world, Eg_shaderStorageImageExtendedFormats);
	ECS_TAG_DEFINE(world, Eg_shaderStorageImageMultisample);
	ECS_TAG_DEFINE(world, Eg_shaderStorageImageReadWithoutFormat);
	ECS_TAG_DEFINE(world, Eg_shaderStorageImageWriteWithoutFormat);
	ECS_TAG_DEFINE(world, Eg_shaderUniformBufferArrayDynamicIndexing);
	ECS_TAG_DEFINE(world, Eg_shaderSampledImageArrayDynamicIndexing);
	ECS_TAG_DEFINE(world, Eg_shaderStorageBufferArrayDynamicIndexing);
	ECS_TAG_DEFINE(world, Eg_shaderStorageImageArrayDynamicIndexing);
	ECS_TAG_DEFINE(world, Eg_shaderClipDistance);
	ECS_TAG_DEFINE(world, Eg_shaderCullDistance);
	ECS_TAG_DEFINE(world, Eg_shaderFloat64);
	ECS_TAG_DEFINE(world, Eg_shaderInt64);
	ECS_TAG_DEFINE(world, Eg_shaderInt16);
	ECS_TAG_DEFINE(world, Eg_shaderResourceResidency);
	ECS_TAG_DEFINE(world, Eg_shaderResourceMinLod);
	ECS_TAG_DEFINE(world, Eg_sparseBinding);
	ECS_TAG_DEFINE(world, Eg_sparseResidencyBuffer);
	ECS_TAG_DEFINE(world, Eg_sparseResidencyImage2D);
	ECS_TAG_DEFINE(world, Eg_sparseResidencyImage3D);
	ECS_TAG_DEFINE(world, Eg_sparseResidency2Samples);
	ECS_TAG_DEFINE(world, Eg_sparseResidency4Samples);
	ECS_TAG_DEFINE(world, Eg_sparseResidency8Samples);
	ECS_TAG_DEFINE(world, Eg_sparseResidency16Samples);
	ECS_TAG_DEFINE(world, Eg_sparseResidencyAliased);
	ECS_TAG_DEFINE(world, Eg_variableMultisampleRate);
	ECS_TAG_DEFINE(world, Eg_inheritedQueries);


	//Handles
	ECS_COMPONENT_DEFINE(world, EgVkInstance);
	ECS_COMPONENT_DEFINE(world, EgVkPhysicalDevice);

	ECS_COMPONENT_DEFINE(world, EgVkExtensionProperties);
	ECS_COMPONENT_DEFINE(world, VkApplicationInfo);
	ECS_COMPONENT_DEFINE(world, EgVkLayer);
	ECS_COMPONENT_DEFINE(world, EgVkPresentModeKHR);
	ECS_COMPONENT_DEFINE(world, EgVkPhysicalDeviceProperties);
	ECS_COMPONENT_DEFINE(world, EgVkQueueFamilyProperties);
	ECS_COMPONENT_DEFINE(world, EgVkSurfaceFormatKHR);





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


	{
		VkExtensionSwapchain = ecs_entity_init(world, &(ecs_entity_desc_t)
		{
		.name = VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		.add = {EgVkExtension}
		});
	}
}










