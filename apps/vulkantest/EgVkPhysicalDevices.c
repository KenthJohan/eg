#include "EgVkPhysicalDevices.h"
#include "EgVk.h"
#include "EgTypes.h"
#include "EgPlatform.h"
#include "EgLogs.h"
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
		switch(items[i])
		{
		case VK_PRESENT_MODE_IMMEDIATE_KHR:
			ecs_add(world, r, Eg_VK_PRESENT_MODE_IMMEDIATE_KHR);
			break;
		case VK_PRESENT_MODE_MAILBOX_KHR:
			ecs_add(world, r, Eg_VK_PRESENT_MODE_MAILBOX_KHR);
			break;
		case VK_PRESENT_MODE_FIFO_KHR:
			ecs_add(world, r, Eg_VK_PRESENT_MODE_FIFO_KHR);
			break;
		case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
			ecs_add(world, r, Eg_VK_PRESENT_MODE_FIFO_RELAXED_KHR);
			break;
		case VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR:
			ecs_add(world, r, Eg_VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR);
			break;
		case VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR:
			ecs_add(world, r, Eg_VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR);
			break;
		case VK_PRESENT_MODE_MAX_ENUM_KHR:
			ecs_add(world, r, Eg_VK_PRESENT_MODE_MAX_ENUM_KHR);
			break;
		}
	}
	ecs_os_free(items);
}


void populate_VkPhysicalDevice(ecs_world_t * world, ecs_entity_t parent, VkInstance instance, VkSurfaceKHR surface)
{
	uint32_t count = 0;
	vkEnumeratePhysicalDevices(instance, &count, NULL);
	VkPhysicalDevice * devices = ecs_os_malloc_n(VkPhysicalDevice, count);
	vkEnumeratePhysicalDevices(instance, &count, devices);
	EG_EVENT_STRF(world, EgVkLogVerbose, "vkEnumeratePhysicalDevices : %i\n", count);
	for (uint32_t i = 0; i < count; ++i)
	{
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(devices[i], &props);
		char name[100];
		snprintf(name, 100, "gpu%i", props.deviceID);
		ecs_entity_t r = ecs_new_entity(world, name);
		ecs_doc_set_color(world, r, "#706546");
		ecs_add_pair(world, r, EcsChildOf, parent);
		ecs_entity_t scope = ecs_set_scope(world, r);
		ecs_set(world, r, EgVkPhysicalDevice, {devices[i]});
		ecs_set_ptr(world, r, EgVkPhysicalDeviceProperties, &props);
		ecs_doc_set_name(world, r, props.deviceName);
		populate_VkSurfaceFormatKHR(world, r, devices[i], surface);
		populate_VkPresentModeKHR(world, r, devices[i], surface);
		ecs_set_scope(world, scope);
	}
	ecs_os_free(devices);
}


static void Observer_populate_VkPhysicalDevice(ecs_iter_t *it)
{
	EgVkInstance * instance = ecs_field(it, EgVkInstance, 1);
	EgVkSurfaceKHR * surface = ecs_field(it, EgVkSurfaceKHR, 2);
	for (int i = 0; i < it->count; i ++)
	{
		populate_VkPhysicalDevice(it->world, it->entities[i], instance[i].instance, surface[i].surface);
	}
}



void set_features(ecs_world_t * world, ecs_entity_t r, VkPhysicalDevice device)
{
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	if(supportedFeatures.robustBufferAccess)                      {ecs_add(world, r, EgVkPhysicalDeviceFeature_robustBufferAccess                        );}
	if(supportedFeatures.fullDrawIndexUint32)                     {ecs_add(world, r, EgVkPhysicalDeviceFeature_fullDrawIndexUint32                       );}
	if(supportedFeatures.imageCubeArray)                          {ecs_add(world, r, EgVkPhysicalDeviceFeature_imageCubeArray                            );}
	if(supportedFeatures.independentBlend)                        {ecs_add(world, r, EgVkPhysicalDeviceFeature_independentBlend                          );}
	if(supportedFeatures.geometryShader)                          {ecs_add(world, r, EgVkPhysicalDeviceFeature_geometryShader                            );}
	if(supportedFeatures.tessellationShader)                      {ecs_add(world, r, EgVkPhysicalDeviceFeature_tessellationShader                        );}
	if(supportedFeatures.sampleRateShading)                       {ecs_add(world, r, EgVkPhysicalDeviceFeature_sampleRateShading                         );}
	if(supportedFeatures.dualSrcBlend)                            {ecs_add(world, r, EgVkPhysicalDeviceFeature_dualSrcBlend                              );}
	if(supportedFeatures.logicOp)                                 {ecs_add(world, r, EgVkPhysicalDeviceFeature_logicOp                                   );}
	if(supportedFeatures.multiDrawIndirect)                       {ecs_add(world, r, EgVkPhysicalDeviceFeature_multiDrawIndirect                         );}
	if(supportedFeatures.drawIndirectFirstInstance)               {ecs_add(world, r, EgVkPhysicalDeviceFeature_drawIndirectFirstInstance                 );}
	if(supportedFeatures.depthClamp)                              {ecs_add(world, r, EgVkPhysicalDeviceFeature_depthClamp                                );}
	if(supportedFeatures.depthBiasClamp)                          {ecs_add(world, r, EgVkPhysicalDeviceFeature_depthBiasClamp                            );}
	if(supportedFeatures.fillModeNonSolid)                        {ecs_add(world, r, EgVkPhysicalDeviceFeature_fillModeNonSolid                          );}
	if(supportedFeatures.depthBounds)                             {ecs_add(world, r, EgVkPhysicalDeviceFeature_depthBounds                               );}
	if(supportedFeatures.wideLines)                               {ecs_add(world, r, EgVkPhysicalDeviceFeature_wideLines                                 );}
	if(supportedFeatures.largePoints)                             {ecs_add(world, r, EgVkPhysicalDeviceFeature_largePoints                               );}
	if(supportedFeatures.alphaToOne)                              {ecs_add(world, r, EgVkPhysicalDeviceFeature_alphaToOne                                );}
	if(supportedFeatures.multiViewport)                           {ecs_add(world, r, EgVkPhysicalDeviceFeature_multiViewport                             );}
	if(supportedFeatures.samplerAnisotropy)                       {ecs_add(world, r, EgVkPhysicalDeviceFeature_samplerAnisotropy                         );}
	if(supportedFeatures.textureCompressionETC2)                  {ecs_add(world, r, EgVkPhysicalDeviceFeature_textureCompressionETC2                    );}
	if(supportedFeatures.textureCompressionASTC_LDR)              {ecs_add(world, r, EgVkPhysicalDeviceFeature_textureCompressionASTC_LDR                );}
	if(supportedFeatures.textureCompressionBC)                    {ecs_add(world, r, EgVkPhysicalDeviceFeature_textureCompressionBC                      );}
	if(supportedFeatures.occlusionQueryPrecise)                   {ecs_add(world, r, EgVkPhysicalDeviceFeature_occlusionQueryPrecise                     );}
	if(supportedFeatures.pipelineStatisticsQuery)                 {ecs_add(world, r, EgVkPhysicalDeviceFeature_pipelineStatisticsQuery                   );}
	if(supportedFeatures.vertexPipelineStoresAndAtomics)          {ecs_add(world, r, EgVkPhysicalDeviceFeature_vertexPipelineStoresAndAtomics            );}
	if(supportedFeatures.fragmentStoresAndAtomics)                {ecs_add(world, r, EgVkPhysicalDeviceFeature_fragmentStoresAndAtomics                  );}
	if(supportedFeatures.shaderTessellationAndGeometryPointSize)  {ecs_add(world, r, EgVkPhysicalDeviceFeature_shaderTessellationAndGeometryPointSize    );}
	if(supportedFeatures.shaderImageGatherExtended)               {ecs_add(world, r, EgVkPhysicalDeviceFeature_shaderImageGatherExtended                 );}
	if(supportedFeatures.shaderStorageImageExtendedFormats)       {ecs_add(world, r, EgVkPhysicalDeviceFeature_shaderStorageImageExtendedFormats         );}
	if(supportedFeatures.shaderStorageImageMultisample)           {ecs_add(world, r, EgVkPhysicalDeviceFeature_shaderStorageImageMultisample             );}
	if(supportedFeatures.shaderStorageImageReadWithoutFormat)     {ecs_add(world, r, EgVkPhysicalDeviceFeature_shaderStorageImageReadWithoutFormat       );}
	if(supportedFeatures.shaderStorageImageWriteWithoutFormat)    {ecs_add(world, r, EgVkPhysicalDeviceFeature_shaderStorageImageWriteWithoutFormat      );}
	if(supportedFeatures.shaderUniformBufferArrayDynamicIndexing) {ecs_add(world, r, EgVkPhysicalDeviceFeature_shaderUniformBufferArrayDynamicIndexing   );}
	if(supportedFeatures.shaderSampledImageArrayDynamicIndexing)  {ecs_add(world, r, EgVkPhysicalDeviceFeature_shaderSampledImageArrayDynamicIndexing    );}
	if(supportedFeatures.shaderStorageBufferArrayDynamicIndexing) {ecs_add(world, r, EgVkPhysicalDeviceFeature_shaderStorageBufferArrayDynamicIndexing   );}
	if(supportedFeatures.shaderStorageImageArrayDynamicIndexing)  {ecs_add(world, r, EgVkPhysicalDeviceFeature_shaderStorageImageArrayDynamicIndexing    );}
	if(supportedFeatures.shaderClipDistance)                      {ecs_add(world, r, EgVkPhysicalDeviceFeature_shaderClipDistance                        );}
	if(supportedFeatures.shaderCullDistance)                      {ecs_add(world, r, EgVkPhysicalDeviceFeature_shaderCullDistance                        );}
	if(supportedFeatures.shaderFloat64)                           {ecs_add(world, r, EgVkPhysicalDeviceFeature_shaderFloat64                             );}
	if(supportedFeatures.shaderInt64)                             {ecs_add(world, r, EgVkPhysicalDeviceFeature_shaderInt64                               );}
	if(supportedFeatures.shaderInt16)                             {ecs_add(world, r, EgVkPhysicalDeviceFeature_shaderInt16                               );}
	if(supportedFeatures.shaderResourceResidency)                 {ecs_add(world, r, EgVkPhysicalDeviceFeature_shaderResourceResidency                   );}
	if(supportedFeatures.shaderResourceMinLod)                    {ecs_add(world, r, EgVkPhysicalDeviceFeature_shaderResourceMinLod                      );}
	if(supportedFeatures.sparseBinding)                           {ecs_add(world, r, EgVkPhysicalDeviceFeature_sparseBinding                             );}
	if(supportedFeatures.sparseResidencyBuffer)                   {ecs_add(world, r, EgVkPhysicalDeviceFeature_sparseResidencyBuffer                     );}
	if(supportedFeatures.sparseResidencyImage2D)                  {ecs_add(world, r, EgVkPhysicalDeviceFeature_sparseResidencyImage2D                    );}
	if(supportedFeatures.sparseResidencyImage3D)                  {ecs_add(world, r, EgVkPhysicalDeviceFeature_sparseResidencyImage3D                    );}
	if(supportedFeatures.sparseResidency2Samples)                 {ecs_add(world, r, EgVkPhysicalDeviceFeature_sparseResidency2Samples                   );}
	if(supportedFeatures.sparseResidency4Samples)                 {ecs_add(world, r, EgVkPhysicalDeviceFeature_sparseResidency4Samples                   );}
	if(supportedFeatures.sparseResidency8Samples)                 {ecs_add(world, r, EgVkPhysicalDeviceFeature_sparseResidency8Samples                   );}
	if(supportedFeatures.sparseResidency16Samples)                {ecs_add(world, r, EgVkPhysicalDeviceFeature_sparseResidency16Samples                  );}
	if(supportedFeatures.sparseResidencyAliased)                  {ecs_add(world, r, EgVkPhysicalDeviceFeature_sparseResidencyAliased                    );}
	if(supportedFeatures.variableMultisampleRate)                 {ecs_add(world, r, EgVkPhysicalDeviceFeature_variableMultisampleRate                   );}
	if(supportedFeatures.inheritedQueries)                        {ecs_add(world, r, EgVkPhysicalDeviceFeature_inheritedQueries                          );}
}

static void Observer_set_features(ecs_iter_t *it)
{
	EgVkPhysicalDevice * d = ecs_field(it, EgVkPhysicalDevice, 1);
	for (int i = 0; i < it->count; i ++)
	{
		set_features(it->world, it->entities[i], d[i].device);
	}
}



void EgVkPhysicalDevicesImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgVkPhysicalDevices);
	ECS_IMPORT(world, EgVk);
	ECS_IMPORT(world, EgTypes);
	ecs_set_name_prefix(world, "EgVk");



	ECS_OBSERVER(world, Observer_populate_VkPhysicalDevice, EcsOnSet, EgVkInstance, EgVkSurfaceKHR);
	ECS_OBSERVER(world, Observer_set_features, EcsOnSet, EgVkPhysicalDevice);
}















