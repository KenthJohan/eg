#pragma once
#include <flecs.h>

void EgImguiImport(ecs_world_t *world);

ecs_query_t * egimgui_query1(ecs_world_t *world);
void egimgui_progress1(ecs_world_t *world, ecs_query_t *q);