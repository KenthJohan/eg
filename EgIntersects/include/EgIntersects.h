#pragma once

#include <flecs.h>

typedef struct {
	bool overlap;
} EgIntersectsOverlap;

extern ECS_COMPONENT_DECLARE(EgIntersectsOverlap);

void EgIntersectsImport(ecs_world_t *world);
