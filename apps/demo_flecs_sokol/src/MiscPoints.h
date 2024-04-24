#pragma once
#include <flecs.h>

#include "points.h"

typedef struct {
	points_storage_t storage;
} PointsBuffer;

extern ECS_COMPONENT_DECLARE(PointsBuffer);

void MiscPointsImport(ecs_world_t *world);