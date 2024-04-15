#pragma once
#include "microui.h"
#include <flecs.h>

void gui_can_progress(mu_Context *ctx, ecs_world_t *world, ecs_query_t *q);
void gui_can_progress2(mu_Context *ctx, ecs_world_t *world, ecs_query_t *q);