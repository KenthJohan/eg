#pragma once
#include <flecs.h>

ecs_query_t *gui_canids_query(ecs_world_t *world);
void gui_canids_progress(ecs_world_t *world, ecs_query_t *q);