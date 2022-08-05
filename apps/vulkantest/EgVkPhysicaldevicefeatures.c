#include "EgVkPhysicaldevicefeatures.h"
#include "EgTypes.h"
#include "EgVk.h"
#include <stdio.h>



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

void EgVkPhysicaldevicefeaturesImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgVkPhysicaldevicefeatures);
	ecs_set_name_prefix(world, "EgVk");

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

	ECS_OBSERVER(world, Observer_set_features, EcsOnSet, EgVkPhysicalDevice);
}










