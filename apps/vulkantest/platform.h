#pragma once
#include "flecs.h"


#ifdef __cplusplus
extern "C" {
#endif

void platform_get_required_extension_names(ecs_world_t * world, ecs_entity_t e);

#ifdef __cplusplus
}
#endif
