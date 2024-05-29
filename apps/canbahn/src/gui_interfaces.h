#pragma once
#include <flecs.h>


void gui_interfaces_progress(ecs_world_t *world, ecs_query_t *q);
ecs_query_t * gui_interfaces_query(ecs_world_t *world);