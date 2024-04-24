#pragma once
#include <stdio.h>
#include <egshapes.h>
#include <egcomponents.h>
#include <sokol_gfx.h>



typedef enum {
	SSHAPE_TORUS,
	SSHAPE_BOX,
	SSHAPE_CYLINDER,
	SSHAPE_SPHERE,
	SSHAPE_COUNT,
} sshape_t;

void ShapeBuffer_append(ShapeBuffer *b, ShapeElement *el, sshape_t shape, void *data, uint32_t color);

void ShapeBuffer_upload(ShapeBuffer *storage);


void ShapeBuffer_reset(ShapeBuffer *storage);