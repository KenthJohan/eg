#pragma once

#include <flecs.h>
#include <EgSpatials.h>
#include <EgShapes.h>

typedef enum {
	EgUiDirectionNone,
	EgUiDirectionRight,
	EgUiDirectionLeft,
	EgUiDirectionUp,
	EgUiDirectionDown,
} EgUiDirection;

typedef struct {
	EgUiDirection direction; // The primary direction of the UI flow
	EgUiDirection wrap;      // The direction to wrap the UI flow when reaching the end
	float         cursor_primary;
	float         cursor_wrap;
	float         line_wrap_extent;
	bool          line_has_children;
} EgUiFlow;

typedef struct {
	int32_t row;
	int32_t col;
} EgUiCell;

typedef struct {
	ecs_map_t         rows_height; // Resolved row heights this frame, key = row index
	ecs_map_t         cols_width;  // Resolved column widths this frame, key = column index
	EgShapesRectangle total_space; // The total space occupied by the table
	float             row_gap;     // Spacing between rows
	float             col_gap;     // Spacing between columns
	int32_t           row_count;   // Highest row index seen + 1
	int32_t           col_count;   // Highest column index seen + 1
} EgUiTable;

typedef struct {
	uint32_t key; // The key code associated with the button
	bool hovered; // Indicates if the button is currently hovered over
	bool held; // Indicates if the button is currently held down
} EgUiButton;

enum {
	EG_UI_EDGE_LEFT   = 1,
	EG_UI_EDGE_RIGHT  = 2,
	EG_UI_EDGE_BOTTOM = 4,
	EG_UI_EDGE_TOP    = 8,
};

typedef struct {
	uint32_t key;      // Mouse button used to drag
	float    grab;     // Edge hit width in rectangle-local units
	float    min_w;    // Minimum width while resizing
	float    min_h;    // Minimum height while resizing
	uint8_t  edge;     // Hovered or active edges, EG_UI_EDGE_* bits
	bool     dragging; // True while an edge is being dragged
	bool     was_held; // Button state of the previous frame
	float    offset_x; // Mouse offset from the grabbed edge at press
	float    offset_y;
} EgUiResizable;

typedef enum {
	EgUiAnchorKindMiddle,
	EgUiAnchorKindTopLeft,
	EgUiAnchorKindTop,
	EgUiAnchorKindTopRight,
	EgUiAnchorKindLeft,
	EgUiAnchorKindRight,
	EgUiAnchorKindBottomLeft,
	EgUiAnchorKindBottom,
	EgUiAnchorKindBottomRight,
} EgUiAnchorKind;

typedef struct {
	EgUiAnchorKind parent; // Point on the parent rectangle
	EgUiAnchorKind pivot;  // Point on this rectangle placed on the parent point
	float          x;      // Offset from the parent point, parent space
	float          y;
} EgUiAnchor;

typedef struct {
	float    thickness; // Separator line thickness
	uint32_t color;     // 0xAARRGGBB
	bool     outer;     // Also draw the outline of the table
} EgUiTableGrid;

extern ECS_COMPONENT_DECLARE(EgUiTableGrid);
extern ECS_COMPONENT_DECLARE(EgUiResizable);
extern ECS_COMPONENT_DECLARE(EgUiAnchorKind);
extern ECS_COMPONENT_DECLARE(EgUiAnchor);
extern ECS_COMPONENT_DECLARE(EgUiFlow);
extern ECS_COMPONENT_DECLARE(EgUiDirection);
extern ECS_COMPONENT_DECLARE(EgUiTable);
extern ECS_COMPONENT_DECLARE(EgUiCell);
extern ECS_COMPONENT_DECLARE(EgUiButton);

void EgUiImport(ecs_world_t *world);
