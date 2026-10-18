#pragma once

#include <flecs.h>

#include "EgUiCommon.h"

typedef struct {
	int32_t dummy;
} EgUiFreeformsLayout;

typedef struct {
	EgUiAt parent;
	EgUiAt pivot;
	float                   x;
	float                   y;
} EgUiFreeformsAnchor;

extern ECS_COMPONENT_DECLARE(EgUiFreeformsLayout);
extern ECS_COMPONENT_DECLARE(EgUiAt);
extern ECS_COMPONENT_DECLARE(EgUiFreeformsAnchor);

void EgUiFreeformsImport(ecs_world_t *world);
