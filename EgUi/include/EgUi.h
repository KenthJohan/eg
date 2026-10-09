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
	EgUiDirection direction;         // Direction children advance along the primary axis
	EgUiDirection wrap;              // Direction the flow advances when starting a new line
	float         cursor_primary;    // Parent-local position along the primary axis
	float         cursor_wrap;       // Parent-local position along the wrap axis
	float         line_wrap_extent;  // Largest child size along the wrap axis on this line
	bool          line_has_children; // Whether the current line already contains a child
} EgUiFlow;

typedef struct {
	float left;   // Signed distance from the child's left edge to the parent's left edge
	float right;  // Signed distance from the parent's right edge to the child's right edge
	float bottom; // Signed distance from the child's bottom edge to the parent's bottom edge
	float top;    // Signed distance from the parent's top edge to the child's top edge
} EgUiParentClearance;

typedef struct {
	uint32_t key;     // The key code associated with the button
	bool     hovered; // Indicates if the button is currently hovered over
	bool     held;    // Indicates if the button is currently held down
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
	float    req_dw;   // Requested width change this frame, consumed by the apply systems
	float    req_dh;   // Requested height change this frame, consumed by the apply systems
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

extern ECS_COMPONENT_DECLARE(EgUiResizable);
extern ECS_COMPONENT_DECLARE(EgUiAnchorKind);
extern ECS_COMPONENT_DECLARE(EgUiAnchor);
extern ECS_COMPONENT_DECLARE(EgUiFlow);
extern ECS_COMPONENT_DECLARE(EgUiParentClearance);
extern ECS_COMPONENT_DECLARE(EgUiDirection);
extern ECS_COMPONENT_DECLARE(EgUiButton);
extern ECS_TAG_DECLARE(EgUiFlowUnplaced);
extern ECS_TAG_DECLARE(EgUiContainer); // Parent that does not position its children

void EgUiImport(ecs_world_t *world);
