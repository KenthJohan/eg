#pragma once
#include <stdio.h>
#include <eg/Components.h>
#include <sokol/sokol_gfx.h>



typedef enum {
	SSHAPE_TORUS,
	SSHAPE_BOX,
	SSHAPE_CYLINDER,
	SSHAPE_COUNT,
} sshape_t;

void ShapeBuffer_append(ShapeBuffer *b, ShapeElement *el, sshape_t shape, void *data);

void ShapeBuffer_upload(ShapeBuffer *storage);


void ShapeBuffer_reset(ShapeBuffer *storage);