#pragma once

#include <flecs.h>

typedef enum {
	EgUiAtMiddle,
	EgUiAtTopLeft,
	EgUiAtTop,
	EgUiAtTopRight,
	EgUiAtLeft,
	EgUiAtRight,
	EgUiAtBottomLeft,
	EgUiAtBottom,
	EgUiAtBottomRight,
} EgUiAt;

typedef enum {
	EgUiDirNone,
	EgUiDirRight,
	EgUiDirLeft,
	EgUiDirUp,
	EgUiDirDown,
} EgUiDir;

typedef struct {
	float left;   // Signed distance from the child's left edge to the parent's left edge
	float right;  // Signed distance from the parent's right edge to the child's right edge
	float bottom; // Signed distance from the child's bottom edge to the parent's bottom edge
	float top;    // Signed distance from the parent's top edge to the child's top edge
} EgUiParentClearance;
