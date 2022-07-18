#include "EgVk.h"
#include <stdio.h>









ECS_COMPONENT_DECLARE(VkApplicationInfo);

ECS_COMPONENT_DECLARE(EgVkPhysicalDevice);
ECS_COMPONENT_DECLARE(EgVkPresentModeKHR);
ECS_COMPONENT_DECLARE(EgVkPhysicalDeviceProperties);
ECS_COMPONENT_DECLARE(EgVkQueueFamilyProperties);
ECS_COMPONENT_DECLARE(EgVkSurfaceFormatKHR);



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
	ECS_MODULE(world, EgVk);
	ecs_set_name_prefix(world, "Eg");

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


	ECS_COMPONENT_DEFINE(world, VkApplicationInfo);
	ECS_COMPONENT_DEFINE(world, EgVkPhysicalDevice);
	ECS_COMPONENT_DEFINE(world, EgVkPresentModeKHR);
	ECS_COMPONENT_DEFINE(world, EgVkPhysicalDeviceProperties);
	ECS_COMPONENT_DEFINE(world, EgVkQueueFamilyProperties);
	ECS_COMPONENT_DEFINE(world, EgVkSurfaceFormatKHR);


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
	.entity.entity = ecs_id(VkApplicationInfo),
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
	.entity.entity = ecs_id(EgVkPresentModeKHR),
	.members = {
	{ .name = "mode", .type = ecs_id(ecs_u32_t) }
	}
	});

	ecs_struct_init(world, &(ecs_struct_desc_t){
	.entity.entity = ecs_id(EgVkPhysicalDeviceProperties),
	.members = {
	{ .name = "apiVersion", .type = ecs_id(ecs_u32_t) },
	{ .name = "driverVersion", .type = ecs_id(ecs_u32_t) },
	{ .name = "vendorID", .type = ecs_id(ecs_u32_t) },
	{ .name = "deviceID", .type = ecs_id(ecs_u32_t) },
	}
	});

	ecs_struct_init(world, &(ecs_struct_desc_t){
	.entity.entity = ecs_id(EgVkQueueFamilyProperties),
	.members = {
	{ .name = "queueFlags", .type = ecs_id(ecs_u32_t) },
	{ .name = "queueCount", .type = ecs_id(ecs_u32_t) },
	{ .name = "timestampValidBits", .type = ecs_id(ecs_u32_t) }
	}
	});

	ecs_struct_init(world, &(ecs_struct_desc_t){
	.entity.entity = ecs_id(EgVkSurfaceFormatKHR),
	.members = {
	{ .name = "format", .type = ecs_id(ecs_i32_t) },
	{ .name = "colorSpace", .type = ecs_id(ecs_i32_t) },
	}
	});

}





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





void populate_VkExtensionProperties(ecs_world_t * world, ecs_entity_t parent, VkPhysicalDevice device)
{
	uint32_t count = 0;
	vkEnumerateDeviceExtensionProperties(device, NULL, &count, NULL);
	VkExtensionProperties * items = ecs_os_malloc_n(VkExtensionProperties, count);
	vkEnumerateDeviceExtensionProperties(device, NULL, &count, items);
	for (uint32_t i = 0; i < count; ++i)
	{
		ecs_entity_t r = ecs_entity_init(world, &(ecs_entity_desc_t){
		.name = items[i].extensionName
		});
		ecs_add_id(world, parent, r);
	}
	ecs_os_free(items);
}







void populate_VkQueueFamilyProperties(ecs_world_t * world, ecs_entity_t parent, VkPhysicalDevice device, VkSurfaceKHR surface)
{
	uint32_t count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, NULL);
	VkQueueFamilyProperties * items = ecs_os_malloc_n(VkQueueFamilyProperties, count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, items);
	for (uint32_t i = 0; i < count; ++i)
	{
		ecs_entity_t r = ecs_new(world, 0);
		ecs_add_pair(world, r, EcsChildOf, parent);
		ecs_doc_set_name(world, r, "QueueFamily");
		ecs_set_ptr(world, r, EgVkQueueFamilyProperties, items + i);
		if (items[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			ecs_add(world, parent, Eg_VK_QUEUE_GRAPHICS_BIT);
			ecs_add(world, r, Eg_VK_QUEUE_GRAPHICS_BIT);
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






void populate_VkPhysicalDevice(ecs_world_t * world, VkInstance instance, VkSurfaceKHR surface)
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
		ecs_entity_t r = ecs_new_entity(world, name);
		ecs_set(world, r, EgVkPhysicalDevice, {devices[i]});
		ecs_set_ptr(world, r, EgVkPhysicalDeviceProperties, &props);
		ecs_doc_set_name(world, r, props.deviceName);
		populate_VkQueueFamilyProperties(world, r, devices[i], surface);
		populate_VkExtensionProperties(world, r, devices[i]);
		populate_VkSurfaceFormatKHR(world, r, devices[i], surface);
		populate_VkPresentModeKHR(world, r, devices[i], surface);
		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(devices[i], &supportedFeatures);

		if(supportedFeatures.robustBufferAccess)                             {ecs_add(world, r, Eg_robustBufferAccess                        );}
		if(supportedFeatures.fullDrawIndexUint32)                            {ecs_add(world, r, Eg_fullDrawIndexUint32                       );}
		if(supportedFeatures.imageCubeArray)                                 {ecs_add(world, r, Eg_imageCubeArray                            );}
		if(supportedFeatures.independentBlend)                               {ecs_add(world, r, Eg_independentBlend                          );}
		if(supportedFeatures.geometryShader)                                 {ecs_add(world, r, Eg_geometryShader                            );}
		if(supportedFeatures.tessellationShader)                             {ecs_add(world, r, Eg_tessellationShader                        );}
		if(supportedFeatures.sampleRateShading)                              {ecs_add(world, r, Eg_sampleRateShading                         );}
		if(supportedFeatures.dualSrcBlend)                                   {ecs_add(world, r, Eg_dualSrcBlend                              );}
		if(supportedFeatures.logicOp)                                        {ecs_add(world, r, Eg_logicOp                                   );}
		if(supportedFeatures.multiDrawIndirect)                              {ecs_add(world, r, Eg_multiDrawIndirect                         );}
		if(supportedFeatures.drawIndirectFirstInstance)                      {ecs_add(world, r, Eg_drawIndirectFirstInstance                 );}
		if(supportedFeatures.depthClamp)                                     {ecs_add(world, r, Eg_depthClamp                                );}
		if(supportedFeatures.depthBiasClamp)                                 {ecs_add(world, r, Eg_depthBiasClamp                            );}
		if(supportedFeatures.fillModeNonSolid)                               {ecs_add(world, r, Eg_fillModeNonSolid                          );}
		if(supportedFeatures.depthBounds)                                    {ecs_add(world, r, Eg_depthBounds                               );}
		if(supportedFeatures.wideLines)                                      {ecs_add(world, r, Eg_wideLines                                 );}
		if(supportedFeatures.largePoints)                                    {ecs_add(world, r, Eg_largePoints                               );}
		if(supportedFeatures.alphaToOne)                                     {ecs_add(world, r, Eg_alphaToOne                                );}
		if(supportedFeatures.multiViewport)                                  {ecs_add(world, r, Eg_multiViewport                             );}
		if(supportedFeatures.samplerAnisotropy)                              {ecs_add(world, r, Eg_samplerAnisotropy                         );}
		if(supportedFeatures.textureCompressionETC2)                         {ecs_add(world, r, Eg_textureCompressionETC2                    );}
		if(supportedFeatures.textureCompressionASTC_LDR)                     {ecs_add(world, r, Eg_textureCompressionASTC_LDR                );}
		if(supportedFeatures.textureCompressionBC)                           {ecs_add(world, r, Eg_textureCompressionBC                      );}
		if(supportedFeatures.occlusionQueryPrecise)                          {ecs_add(world, r, Eg_occlusionQueryPrecise                     );}
		if(supportedFeatures.pipelineStatisticsQuery)                        {ecs_add(world, r, Eg_pipelineStatisticsQuery                   );}
		if(supportedFeatures.vertexPipelineStoresAndAtomics)                 {ecs_add(world, r, Eg_vertexPipelineStoresAndAtomics            );}
		if(supportedFeatures.fragmentStoresAndAtomics)                       {ecs_add(world, r, Eg_fragmentStoresAndAtomics                  );}
		if(supportedFeatures.shaderTessellationAndGeometryPointSize)         {ecs_add(world, r, Eg_shaderTessellationAndGeometryPointSize    );}
		if(supportedFeatures.shaderImageGatherExtended)                      {ecs_add(world, r, Eg_shaderImageGatherExtended                 );}
		if(supportedFeatures.shaderStorageImageExtendedFormats)              {ecs_add(world, r, Eg_shaderStorageImageExtendedFormats         );}
		if(supportedFeatures.shaderStorageImageMultisample)                  {ecs_add(world, r, Eg_shaderStorageImageMultisample             );}
		if(supportedFeatures.shaderStorageImageReadWithoutFormat)            {ecs_add(world, r, Eg_shaderStorageImageReadWithoutFormat       );}
		if(supportedFeatures.shaderStorageImageWriteWithoutFormat)           {ecs_add(world, r, Eg_shaderStorageImageWriteWithoutFormat      );}
		if(supportedFeatures.shaderUniformBufferArrayDynamicIndexing)        {ecs_add(world, r, Eg_shaderUniformBufferArrayDynamicIndexing   );}
		if(supportedFeatures.shaderSampledImageArrayDynamicIndexing)         {ecs_add(world, r, Eg_shaderSampledImageArrayDynamicIndexing    );}
		if(supportedFeatures.shaderStorageBufferArrayDynamicIndexing)        {ecs_add(world, r, Eg_shaderStorageBufferArrayDynamicIndexing   );}
		if(supportedFeatures.shaderStorageImageArrayDynamicIndexing)         {ecs_add(world, r, Eg_shaderStorageImageArrayDynamicIndexing    );}
		if(supportedFeatures.shaderClipDistance)                             {ecs_add(world, r, Eg_shaderClipDistance                        );}
		if(supportedFeatures.shaderCullDistance)                             {ecs_add(world, r, Eg_shaderCullDistance                        );}
		if(supportedFeatures.shaderFloat64)                                  {ecs_add(world, r, Eg_shaderFloat64                             );}
		if(supportedFeatures.shaderInt64)                                    {ecs_add(world, r, Eg_shaderInt64                               );}
		if(supportedFeatures.shaderInt16)                                    {ecs_add(world, r, Eg_shaderInt16                               );}
		if(supportedFeatures.shaderResourceResidency)                        {ecs_add(world, r, Eg_shaderResourceResidency                   );}
		if(supportedFeatures.shaderResourceMinLod)                           {ecs_add(world, r, Eg_shaderResourceMinLod                      );}
		if(supportedFeatures.sparseBinding)                                  {ecs_add(world, r, Eg_sparseBinding                             );}
		if(supportedFeatures.sparseResidencyBuffer)                          {ecs_add(world, r, Eg_sparseResidencyBuffer                     );}
		if(supportedFeatures.sparseResidencyImage2D)                         {ecs_add(world, r, Eg_sparseResidencyImage2D                    );}
		if(supportedFeatures.sparseResidencyImage3D)                         {ecs_add(world, r, Eg_sparseResidencyImage3D                    );}
		if(supportedFeatures.sparseResidency2Samples)                        {ecs_add(world, r, Eg_sparseResidency2Samples                   );}
		if(supportedFeatures.sparseResidency4Samples)                        {ecs_add(world, r, Eg_sparseResidency4Samples                   );}
		if(supportedFeatures.sparseResidency8Samples)                        {ecs_add(world, r, Eg_sparseResidency8Samples                   );}
		if(supportedFeatures.sparseResidency16Samples)                       {ecs_add(world, r, Eg_sparseResidency16Samples                  );}
		if(supportedFeatures.sparseResidencyAliased)                         {ecs_add(world, r, Eg_sparseResidencyAliased                    );}
		if(supportedFeatures.variableMultisampleRate)                        {ecs_add(world, r, Eg_variableMultisampleRate                   );}
		if(supportedFeatures.inheritedQueries)                               {ecs_add(world, r, Eg_inheritedQueries                          );}

	}
	ecs_os_free(devices);
}
