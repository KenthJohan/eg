#pragma once
#include <stdio.h>
#include <egshapes.h>
#include <egbase.h>
#include <sokol_gfx.h>
#include "MyGraphics.h"



typedef enum {
	SSHAPE_TORUS,
	SSHAPE_BOX,
	SSHAPE_CYLINDER,
	SSHAPE_SPHERE,
	SSHAPE_COUNT,
} sshape_t;

void ShapeBuffer_append(EgBaseShapeBuffer *b, MyGraphicsDrawCommand *el, sshape_t shape, void *data, uint32_t color);

void ShapeBuffer_upload(EgBaseShapeBuffer *storage);


void ShapeBuffer_reset(EgBaseShapeBuffer *storage);