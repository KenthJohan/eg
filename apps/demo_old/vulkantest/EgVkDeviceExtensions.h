#pragma once

#include "flecs.h"
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif


extern ECS_COMPONENT_DECLARE(EgVkDeviceExtensions);

void EgVkDeviceExtensionsImport(ecs_world_t *world);


#ifdef __cplusplus
}
#endif