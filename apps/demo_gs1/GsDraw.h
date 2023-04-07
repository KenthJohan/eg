#pragma once
#include "flecs.h"

typedef struct {
	void * ptr; // Must be of type (gs_immediate_draw_t *)
} GsImmediateDraw;

extern ECS_COMPONENT_DECLARE(GsImmediateDraw);

void GsDrawImport(ecs_world_t *world);
