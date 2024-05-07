#pragma once
#include <flecs.h>


void gui_plot_progress(ecs_world_t *world, ecs_query_t *q);
ecs_query_t * gui_plot_query(ecs_world_t *world);