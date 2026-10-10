#pragma once

#include <flecs.h>

typedef enum {
	EgUiFreeformsAnchorKindMiddle,
	EgUiFreeformsAnchorKindTopLeft,
	EgUiFreeformsAnchorKindTop,
	EgUiFreeformsAnchorKindTopRight,
	EgUiFreeformsAnchorKindLeft,
	EgUiFreeformsAnchorKindRight,
	EgUiFreeformsAnchorKindBottomLeft,
	EgUiFreeformsAnchorKindBottom,
	EgUiFreeformsAnchorKindBottomRight,
} EgUiFreeformsAnchorKind;

typedef struct {
	int32_t dummy;
} EgUiFreeformsLayout;

typedef struct {
	EgUiFreeformsAnchorKind parent;
	EgUiFreeformsAnchorKind pivot;
	float                   x;
	float                   y;
} EgUiFreeformsAnchor;

extern ECS_COMPONENT_DECLARE(EgUiFreeformsLayout);
extern ECS_COMPONENT_DECLARE(EgUiFreeformsAnchorKind);
extern ECS_COMPONENT_DECLARE(EgUiFreeformsAnchor);

void EgUiFreeformsImport(ecs_world_t *world);
