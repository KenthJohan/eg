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
	ecs_map_t         rows_height; // Key = row index, Value = height which are the maximum gathered from children rectangle height
	ecs_map_t         cols_width;  // Key = column index, Value = width which are the maximum gathered from children rectangle width
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

extern ECS_COMPONENT_DECLARE(EgUiFlow);
extern ECS_COMPONENT_DECLARE(EgUiDirection);
extern ECS_COMPONENT_DECLARE(EgUiTable);
extern ECS_COMPONENT_DECLARE(EgUiCell);
extern ECS_COMPONENT_DECLARE(EgUiButton);

void EgUiImport(ecs_world_t *world);
