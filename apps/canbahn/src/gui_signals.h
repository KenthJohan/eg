#pragma once
#include <flecs.h>


void gui_signals_progress(ecs_world_t *world, ecs_query_t *q);
ecs_query_t * gui_signals_query(ecs_world_t *world);