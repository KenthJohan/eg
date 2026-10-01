#pragma once

#include <flecs.h>
#include <EgSpatials.h>

typedef struct {
	Position2 cursor;
	V2f32     direction;
	float     max;
} EgUiFlow;

extern ECS_COMPONENT_DECLARE(EgUiFlow);

void EgUiImport(ecs_world_t *world);
