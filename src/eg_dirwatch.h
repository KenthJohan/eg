#pragma once
#include <stddef.h>
#include <stdint.h>
#include "flecs.h"


#define EG_DIRWATCH_PATH_LENGTH 1024



void eg_dirwatch_bootstrap(ecs_world_t * world);

ecs_entity_t eg_dirwatch_init(ecs_world_t * world, ecs_entity_t e);
ecs_entity_t eg_dirwatch_add(ecs_world_t * world, ecs_entity_t scope, ecs_entity_t e, char const * path);

