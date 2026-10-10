#pragma once

#include <flecs.h>

enum {
	EG_UI_EDGE_LEFT   = 1,
	EG_UI_EDGE_RIGHT  = 2,
	EG_UI_EDGE_BOTTOM = 4,
	EG_UI_EDGE_TOP    = 8,
};

typedef struct {
	uint32_t key;
	float    min_w;
	float    min_h;
	uint8_t  edge;
	bool     dragging;
	bool     was_held;
	float    offset_x;
	float    offset_y;
	float    req_dw;
	float    req_dh;
} EgUiBoundsResizable;

extern ECS_COMPONENT_DECLARE(EgUiBoundsResizable);

void EgUiBoundsImport(ecs_world_t *world);
