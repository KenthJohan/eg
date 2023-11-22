#pragma once

#include "sokol/sokol_app.h"
#include "sokol/sokol_gfx.h"
#include "sokol/sokol_log.h"
#include "sokol/sokol_glue.h"
#include "sokol/sokol_shape.h"
#include "sokol/sokol_debugtext.h"
#include <assert.h>
#include "draw_shapes.h"
#include "gmath.h"

typedef struct {
	float x;
	float y;
	float z;
} position_t;

enum {
	BOX = 0,
	PLANE,
	SPHERE,
	CYLINDER,
	TORUS,
	NUM_SHAPES
};

typedef struct {
	float draw_mode;
	uint8_t _pad_4[12];
	float mvp[4 * 4];
} vs_params_t;


typedef struct {
	sg_pipeline pip;
	sg_buffer vbuf;
	sg_buffer ibuf;
	position_t positions[NUM_SHAPES];
	sshape_element_range_t draws[NUM_SHAPES];
	float rx, ry;
	vs_params_t vs_params;
} draw_shapes_t;

void draw_shapes1(draw_shapes_t *state, m4f32 *vp, float a);


void draw_shapes_init(draw_shapes_t * state);

void draw_shapes_frame(draw_shapes_t * state, m4f32 *vp);