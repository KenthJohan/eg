#pragma once
#include "flecs.h"

typedef struct {
	void * ptr; // Must be of type (gs_immediate_draw_t *)
} GsmoduleDraw;

extern ECS_COMPONENT_DECLARE(GsmoduleDraw);

void GsmoduleImport(ecs_world_t *world);
