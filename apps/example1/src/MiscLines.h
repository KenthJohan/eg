#pragma once
#include <flecs.h>

#include "lines.h"

typedef struct {
	lines_storage_t storage;
} LinesBuffer;

extern ECS_COMPONENT_DECLARE(LinesBuffer);

void MiscLinesImport(ecs_world_t *world);