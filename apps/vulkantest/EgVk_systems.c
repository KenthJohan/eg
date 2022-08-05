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





void populate_VkExtensionProperties(ecs_world_t * world, ecs_entity_t parent, VkPhysicalDevice device)
{
	uint32_t count = 0;
	vkEnumerateDeviceExtensionProperties(device, NULL, &count, NULL);
	VkExtensionProperties * items = ecs_os_malloc_n(VkExtensionProperties, count);
	vkEnumerateDeviceExtensionProperties(device, NULL, &count, items);
	for (uint32_t i = 0; i < count; ++i)
	{
		char const * name = items[i].extensionName;
		ecs_entity_t r = ecs_entity_init(world, &(ecs_entity_desc_t){
		.name = name
		});
		//printf("extensionName %s (%lx)\n", name, r);
		ecs_set(world, r, EgVkExtensionProperties, {items[i].specVersion});
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
		char name[100];
		snprintf(name, 100, "qf%i", i);
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
		ecs_set(world, r, EgVkPhysicalDevice, {devices[i]});
		ecs_set_ptr(world, r, EgVkPhysicalDeviceProperties, &props);
		ecs_doc_set_name(world, r, props.deviceName);
		populate_VkQueueFamilyProperties(world, r, devices[i], surface);
		populate_VkExtensionProperties(world, r, devices[i]);
		populate_VkSurfaceFormatKHR(world, r, devices[i], surface);
		populate_VkPresentModeKHR(world, r, devices[i], surface);
		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(devices[i], &supportedFeatures);

		if(supportedFeatures.robustBufferAccess)                      {ecs_add(world, r, Eg_robustBufferAccess                        );}
		if(supportedFeatures.fullDrawIndexUint32)                     {ecs_add(world, r, Eg_fullDrawIndexUint32                       );}
		if(supportedFeatures.imageCubeArray)                          {ecs_add(world, r, Eg_imageCubeArray                            );}
		if(supportedFeatures.independentBlend)                        {ecs_add(world, r, Eg_independentBlend                          );}
		if(supportedFeatures.geometryShader)                          {ecs_add(world, r, Eg_geometryShader                            );}
		if(supportedFeatures.tessellationShader)                      {ecs_add(world, r, Eg_tessellationShader                        );}
		if(supportedFeatures.sampleRateShading)                       {ecs_add(world, r, Eg_sampleRateShading                         );}
		if(supportedFeatures.dualSrcBlend)                            {ecs_add(world, r, Eg_dualSrcBlend                              );}
		if(supportedFeatures.logicOp)                                 {ecs_add(world, r, Eg_logicOp                                   );}
		if(supportedFeatures.multiDrawIndirect)                       {ecs_add(world, r, Eg_multiDrawIndirect                         );}
		if(supportedFeatures.drawIndirectFirstInstance)               {ecs_add(world, r, Eg_drawIndirectFirstInstance                 );}
		if(supportedFeatures.depthClamp)                              {ecs_add(world, r, Eg_depthClamp                                );}
		if(supportedFeatures.depthBiasClamp)                          {ecs_add(world, r, Eg_depthBiasClamp                            );}
		if(supportedFeatures.fillModeNonSolid)                        {ecs_add(world, r, Eg_fillModeNonSolid                          );}
		if(supportedFeatures.depthBounds)                             {ecs_add(world, r, Eg_depthBounds                               );}
		if(supportedFeatures.wideLines)                               {ecs_add(world, r, Eg_wideLines                                 );}
		if(supportedFeatures.largePoints)                             {ecs_add(world, r, Eg_largePoints                               );}
		if(supportedFeatures.alphaToOne)                              {ecs_add(world, r, Eg_alphaToOne                                );}
		if(supportedFeatures.multiViewport)                           {ecs_add(world, r, Eg_multiViewport                             );}
		if(supportedFeatures.samplerAnisotropy)                       {ecs_add(world, r, Eg_samplerAnisotropy                         );}
		if(supportedFeatures.textureCompressionETC2)                  {ecs_add(world, r, Eg_textureCompressionETC2                    );}
		if(supportedFeatures.textureCompressionASTC_LDR)              {ecs_add(world, r, Eg_textureCompressionASTC_LDR                );}
		if(supportedFeatures.textureCompressionBC)                    {ecs_add(world, r, Eg_textureCompressionBC                      );}
		if(supportedFeatures.occlusionQueryPrecise)                   {ecs_add(world, r, Eg_occlusionQueryPrecise                     );}
		if(supportedFeatures.pipelineStatisticsQuery)                 {ecs_add(world, r, Eg_pipelineStatisticsQuery                   );}
		if(supportedFeatures.vertexPipelineStoresAndAtomics)          {ecs_add(world, r, Eg_vertexPipelineStoresAndAtomics            );}
		if(supportedFeatures.fragmentStoresAndAtomics)                {ecs_add(world, r, Eg_fragmentStoresAndAtomics                  );}
		if(supportedFeatures.shaderTessellationAndGeometryPointSize)  {ecs_add(world, r, Eg_shaderTessellationAndGeometryPointSize    );}
		if(supportedFeatures.shaderImageGatherExtended)               {ecs_add(world, r, Eg_shaderImageGatherExtended                 );}
		if(supportedFeatures.shaderStorageImageExtendedFormats)       {ecs_add(world, r, Eg_shaderStorageImageExtendedFormats         );}
		if(supportedFeatures.shaderStorageImageMultisample)           {ecs_add(world, r, Eg_shaderStorageImageMultisample             );}
		if(supportedFeatures.shaderStorageImageReadWithoutFormat)     {ecs_add(world, r, Eg_shaderStorageImageReadWithoutFormat       );}
		if(supportedFeatures.shaderStorageImageWriteWithoutFormat)    {ecs_add(world, r, Eg_shaderStorageImageWriteWithoutFormat      );}
		if(supportedFeatures.shaderUniformBufferArrayDynamicIndexing) {ecs_add(world, r, Eg_shaderUniformBufferArrayDynamicIndexing   );}
		if(supportedFeatures.shaderSampledImageArrayDynamicIndexing)  {ecs_add(world, r, Eg_shaderSampledImageArrayDynamicIndexing    );}
		if(supportedFeatures.shaderStorageBufferArrayDynamicIndexing) {ecs_add(world, r, Eg_shaderStorageBufferArrayDynamicIndexing   );}
		if(supportedFeatures.shaderStorageImageArrayDynamicIndexing)  {ecs_add(world, r, Eg_shaderStorageImageArrayDynamicIndexing    );}
		if(supportedFeatures.shaderClipDistance)                      {ecs_add(world, r, Eg_shaderClipDistance                        );}
		if(supportedFeatures.shaderCullDistance)                      {ecs_add(world, r, Eg_shaderCullDistance                        );}
		if(supportedFeatures.shaderFloat64)                           {ecs_add(world, r, Eg_shaderFloat64                             );}
		if(supportedFeatures.shaderInt64)                             {ecs_add(world, r, Eg_shaderInt64                               );}
		if(supportedFeatures.shaderInt16)                             {ecs_add(world, r, Eg_shaderInt16                               );}
		if(supportedFeatures.shaderResourceResidency)                 {ecs_add(world, r, Eg_shaderResourceResidency                   );}
		if(supportedFeatures.shaderResourceMinLod)                    {ecs_add(world, r, Eg_shaderResourceMinLod                      );}
		if(supportedFeatures.sparseBinding)                           {ecs_add(world, r, Eg_sparseBinding                             );}
		if(supportedFeatures.sparseResidencyBuffer)                   {ecs_add(world, r, Eg_sparseResidencyBuffer                     );}
		if(supportedFeatures.sparseResidencyImage2D)                  {ecs_add(world, r, Eg_sparseResidencyImage2D                    );}
		if(supportedFeatures.sparseResidencyImage3D)                  {ecs_add(world, r, Eg_sparseResidencyImage3D                    );}
		if(supportedFeatures.sparseResidency2Samples)                 {ecs_add(world, r, Eg_sparseResidency2Samples                   );}
		if(supportedFeatures.sparseResidency4Samples)                 {ecs_add(world, r, Eg_sparseResidency4Samples                   );}
		if(supportedFeatures.sparseResidency8Samples)                 {ecs_add(world, r, Eg_sparseResidency8Samples                   );}
		if(supportedFeatures.sparseResidency16Samples)                {ecs_add(world, r, Eg_sparseResidency16Samples                  );}
		if(supportedFeatures.sparseResidencyAliased)                  {ecs_add(world, r, Eg_sparseResidencyAliased                    );}
		if(supportedFeatures.variableMultisampleRate)                 {ecs_add(world, r, Eg_variableMultisampleRate                   );}
		if(supportedFeatures.inheritedQueries)                        {ecs_add(world, r, Eg_inheritedQueries                          );}
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















