#include "EgVk1.h"
#include "EgVk.h"
#include "EgTypes.h"
#include "platform.h"
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
		ecs_entity_t r = ecs_entity_init(world, &(ecs_entity_desc_t){
		.name = items[i].extensionName
		});
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



/*
std::vector<const char*> getRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}
*/

char const ** get_entity_names_from_filter(ecs_world_t * world, ecs_filter_t *f)
{
	ecs_iter_t it;
	it = ecs_filter_iter(world, f);
	int32_t n = ecs_iter_count(&it);
	it = ecs_filter_iter(world, f);
	char const ** r = ecs_os_alloca_n(char const*, n+1);
	int32_t j = 0;
	while (ecs_filter_next(&it))
	{
		for (int32_t i = 0; i < it.count; i ++)
		{
			char const * name = ecs_get_name(world, it.entities[i]);
			printf("qext %s\n", name);
			r[j] = name;
			j++;
		}
	}
	r[j] = NULL;
	return r;
}


void createInstance1(ecs_world_t * world, ecs_entity_t e)
{
	VkApplicationInfo const * app = ecs_get(world, e, VkApplicationInfo);
	ecs_doc_set_name(world, e, app->pApplicationName);
	VkInstanceCreateInfo create = {};
	create.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create.pApplicationInfo = app;

	platform_get_required_extension_names(world, e);
	ecs_entity_t r = ecs_entity_init(world, &(ecs_entity_desc_t)
	{
	.name = VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
	.add = {EgVkExtension}
	});
	ecs_add_id(world, e, r);
	const ecs_type_t *type = ecs_get_type(world, e);
	for (int i = 0; i < type->count; ++i)
	{
		ecs_id_t ext = type->array[i];
		if (ecs_is_valid(world, ext) == false) {continue;}
		if (ecs_has_id(world, ext, EgVkExtension) == false) {continue;}
		printf("ext %s\n", ecs_get_name(world, ext));
	}


	ecs_filter_t *f = ecs_filter_init(world, &(ecs_filter_desc_t){
	  .terms = {
	{ ecs_id(EgVkExtension) },
	{ ecs_id(VkApplicationInfo), .subj.entity = e }
	  }
	});
	char const ** names = get_entity_names_from_filter(world, f);
	ecs_filter_fini(f);
	ecs_os_free(names);

	//VK_EXT_DEBUG_UTILS_EXTENSION_NAME
	//VK_EXT_DEBUG_UTILS_EXTENSION_NAME

	//uint32_t glfwExtensionCount = 0;
	//const char** glfwExtensions;
	//glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	/*
	auto extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = VALIDATION_LAYER_COUNT;
		createInfo.ppEnabledLayerNames = validationLayers;

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
	} else {
		createInfo.enabledLayerCount = 0;

		createInfo.pNext = nullptr;
	}

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}
	*/
	//VkInstance instance;
	//vkCreateInstance(&create, NULL, &instance);
	//ecs_set(world, e, EgVkInstance, {instance});
}



















