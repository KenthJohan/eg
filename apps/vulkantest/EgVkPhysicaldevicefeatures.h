#pragma once

#include "flecs.h"
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif


//typedef struct VkPhysicalDeviceFeatures
extern ECS_DECLARE(Eg_robustBufferAccess);
extern ECS_DECLARE(Eg_fullDrawIndexUint32);
extern ECS_DECLARE(Eg_imageCubeArray);
extern ECS_DECLARE(Eg_independentBlend);
extern ECS_DECLARE(Eg_geometryShader);
extern ECS_DECLARE(Eg_tessellationShader);
extern ECS_DECLARE(Eg_sampleRateShading);
extern ECS_DECLARE(Eg_dualSrcBlend);
extern ECS_DECLARE(Eg_logicOp);
extern ECS_DECLARE(Eg_multiDrawIndirect);
extern ECS_DECLARE(Eg_drawIndirectFirstInstance);
extern ECS_DECLARE(Eg_depthClamp);
extern ECS_DECLARE(Eg_depthBiasClamp);
extern ECS_DECLARE(Eg_fillModeNonSolid);
extern ECS_DECLARE(Eg_depthBounds);
extern ECS_DECLARE(Eg_wideLines);
extern ECS_DECLARE(Eg_largePoints);
extern ECS_DECLARE(Eg_alphaToOne);
extern ECS_DECLARE(Eg_multiViewport);
extern ECS_DECLARE(Eg_samplerAnisotropy);
extern ECS_DECLARE(Eg_textureCompressionETC2);
extern ECS_DECLARE(Eg_textureCompressionASTC_LDR);
extern ECS_DECLARE(Eg_textureCompressionBC);
extern ECS_DECLARE(Eg_occlusionQueryPrecise);
extern ECS_DECLARE(Eg_pipelineStatisticsQuery);
extern ECS_DECLARE(Eg_vertexPipelineStoresAndAtomics);
extern ECS_DECLARE(Eg_fragmentStoresAndAtomics);
extern ECS_DECLARE(Eg_shaderTessellationAndGeometryPointSize);
extern ECS_DECLARE(Eg_shaderImageGatherExtended);
extern ECS_DECLARE(Eg_shaderStorageImageExtendedFormats);
extern ECS_DECLARE(Eg_shaderStorageImageMultisample);
extern ECS_DECLARE(Eg_shaderStorageImageReadWithoutFormat);
extern ECS_DECLARE(Eg_shaderStorageImageWriteWithoutFormat);
extern ECS_DECLARE(Eg_shaderUniformBufferArrayDynamicIndexing);
extern ECS_DECLARE(Eg_shaderSampledImageArrayDynamicIndexing);
extern ECS_DECLARE(Eg_shaderStorageBufferArrayDynamicIndexing);
extern ECS_DECLARE(Eg_shaderStorageImageArrayDynamicIndexing);
extern ECS_DECLARE(Eg_shaderClipDistance);
extern ECS_DECLARE(Eg_shaderCullDistance);
extern ECS_DECLARE(Eg_shaderFloat64);
extern ECS_DECLARE(Eg_shaderInt64);
extern ECS_DECLARE(Eg_shaderInt16);
extern ECS_DECLARE(Eg_shaderResourceResidency);
extern ECS_DECLARE(Eg_shaderResourceMinLod);
extern ECS_DECLARE(Eg_sparseBinding);
extern ECS_DECLARE(Eg_sparseResidencyBuffer);
extern ECS_DECLARE(Eg_sparseResidencyImage2D);
extern ECS_DECLARE(Eg_sparseResidencyImage3D);
extern ECS_DECLARE(Eg_sparseResidency2Samples);
extern ECS_DECLARE(Eg_sparseResidency4Samples);
extern ECS_DECLARE(Eg_sparseResidency8Samples);
extern ECS_DECLARE(Eg_sparseResidency16Samples);
extern ECS_DECLARE(Eg_sparseResidencyAliased);
extern ECS_DECLARE(Eg_variableMultisampleRate);
extern ECS_DECLARE(Eg_inheritedQueries);

void EgVkPhysicaldevicefeaturesImport(ecs_world_t *world);


#ifdef __cplusplus
}
#endif