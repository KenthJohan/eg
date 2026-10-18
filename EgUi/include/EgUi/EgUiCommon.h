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