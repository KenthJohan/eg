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

void ShapeBuffer_append(EgBaseMemory2 *b, MyGraphicsDrawCommand *el, sshape_t shape, void *data, uint32_t color);

void ShapeBuffer_upload(EgBaseMemory2 *storage, EgBaseShapeBuffer * gbuf);


void ShapeBuffer_reset(EgBaseMemory2 *storage);