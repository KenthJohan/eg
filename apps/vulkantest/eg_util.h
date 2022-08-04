#pragma once

#include "flecs.h"

#ifdef __cplusplus
extern "C" {
#endif

char const ** get_entity_names_from_filter(ecs_world_t * world, ecs_filter_t *f);
void const * eg_get_first_from_filter(ecs_world_t * world, ecs_filter_t *f);
void eg_add_all_from_filter(ecs_world_t * world, ecs_entity_t e, ecs_filter_t *f);

#ifdef __cplusplus
}
#endif
