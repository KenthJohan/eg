#include "EgShapedraw.h"

#include <EgBase.h>
#include <EgShapes.h>
#include <EgSpatials.h>
#include <ecsx.h>
#include <stdlib.h>
#include <string.h>

ECS_COMPONENT_DECLARE(EgShapedrawList);

static void EgShapedrawList_ctor(void *ptr, int32_t count, const ecs_type_info_t *ti)
{
	(void)ti;
	memset(ptr, 0, (size_t)count * sizeof(EgShapedrawList));
}

static void EgShapedrawList_dtor(void *ptr, int32_t count, const ecs_type_info_t *ti)
{
	(void)ti;
	EgShapedrawList *l = ptr;
	for (int32_t i = 0; i < count; ++i) {
		free(l[i].data);
		l[i].data = NULL;
	}
}

static void EgShapedrawList_move(void *dst_ptr, void *src_ptr, int32_t count, const ecs_type_info_t *ti)
{
	(void)ti;
	EgShapedrawList *dst = dst_ptr;
	EgShapedrawList *src = src_ptr;
	for (int32_t i = 0; i < count; ++i) {
		free(dst[i].data);
		dst[i]          = src[i];
		src[i].data     = NULL;
		src[i].count    = 0;
		src[i].capacity = 0;
	}
}

static bool EgShapedrawList_Reserve(EgShapedrawList *l, int32_t extra)
{
	int32_t need = l->count + extra;
	if (need <= l->capacity) {
		return true;
	}
	int32_t cap = l->capacity > 0 ? l->capacity : 256;
	while (cap < need) {
		cap *= 2;
	}
	EgShapedrawVertex *data = realloc(l->data, (size_t)cap * sizeof(EgShapedrawVertex));
	if (data == NULL) {
		return false;
	}
	l->data     = data;
	l->capacity = cap;
	return true;
}

void EgShapedrawList_AddTriangle(EgShapedrawList *list, float x0, float y0, float x1, float y1, float x2, float y2, uint32_t color)
{
	if (!EgShapedrawList_Reserve(list, 3)) {
		return;
	}

	uint8_t a = (uint8_t)((color >> 24) & 0xFF);
	uint8_t r = (uint8_t)((color >> 16) & 0xFF);
	uint8_t g = (uint8_t)((color >> 8) & 0xFF);
	uint8_t b = (uint8_t)(color & 0xFF);
	if (a == 0) {
		a = 255;
	}

	const float xy[3][2] = {{x0, y0}, {x1, y1}, {x2, y2}};
	for (int i = 0; i < 3; ++i) {
		EgShapedrawVertex *v = &list->data[list->count++];
		v->position[0]       = xy[i][0];
		v->position[1]       = xy[i][1];
		v->uv[0]             = 0.5f / EG_SHAPEDRAW_ATLAS_WIDTH;
		v->uv[1]             = 0.5f / EG_SHAPEDRAW_ATLAS_HEIGHT;
		v->rgba[0]           = r;
		v->rgba[1]           = g;
		v->rgba[2]           = b;
		v->rgba[3]           = a;
	}
}

// Centered rectangle; (a,b) and (c,d) are the transformed x and y axes, (tx,ty) the origin.
static void EgShapedraw_AddRectangle(EgShapedrawList *l, const EgShapesRectangle *r, float a, float b, float c, float d, float tx, float ty, uint32_t color)
{
	if (r->w <= 0.0f || r->h <= 0.0f) {
		return;
	}

	float hw = r->w * 0.5f;
	float hh = r->h * 0.5f;
	float x[4];
	float y[4];
	// Corner order: (-,-), (+,-), (+,+), (-,+).
	const float sx[4] = {-hw, hw, hw, -hw};
	const float sy[4] = {-hh, -hh, hh, hh};
	for (int i = 0; i < 4; ++i) {
		x[i] = a * sx[i] + c * sy[i] + tx;
		y[i] = b * sx[i] + d * sy[i] + ty;
	}

	EgShapedrawList_AddTriangle(l, x[0], y[0], x[1], y[1], x[2], y[2], color);
	EgShapedrawList_AddTriangle(l, x[0], y[0], x[2], y[2], x[3], y[3], color);
}

static void EgShapedrawList_Reset(ecs_iter_t *it)
{
	EgShapedrawList *l = ecs_field_self(it, EgShapedrawList, 0);
	for (int i = 0; i < it->count; ++i) {
		l[i].count = 0;
	}
}

static void EgShapedrawRectangle_Collect3D(ecs_iter_t *it)
{
	EgShapedrawList         *l   = ecs_field_shared(it, EgShapedrawList, 0);
	EgShapesRectangle const *r   = ecs_field_self(it, EgShapesRectangle, 1);
	WorldTransform4 const   *w   = ecs_field_self(it, WorldTransform4, 2);
	EgBaseColor const       *col = ecs_field_self(it, EgBaseColor, 3);
	for (int i = 0; i < it->count; ++i) {
		uint32_t color = col != NULL ? col[i].color : 0x00FFFF00u;
		EgShapedraw_AddRectangle(l, &r[i], w[i].matrix.c0[0], w[i].matrix.c0[1], w[i].matrix.c1[0], w[i].matrix.c1[1], w[i].matrix.c3[0], w[i].matrix.c3[1], color);
	}
}

static void EgShapedrawRectangle_Collect2D(ecs_iter_t *it)
{
	EgShapedrawList         *l   = ecs_field_shared(it, EgShapedrawList, 0);
	EgShapesRectangle const *r   = ecs_field_self(it, EgShapesRectangle, 1);
	WorldTransform3 const   *w   = ecs_field_self(it, WorldTransform3, 2);
	EgBaseColor const       *col = ecs_field_self(it, EgBaseColor, 3);
	for (int i = 0; i < it->count; ++i) {
		uint32_t color = col != NULL ? col[i].color : 0x00FFFF00u;
		EgShapedraw_AddRectangle(l, &r[i], w[i].matrix.c0[0], w[i].matrix.c0[1], w[i].matrix.c1[0], w[i].matrix.c1[1], w[i].matrix.c2[0], w[i].matrix.c2[1], color);
	}
}

static void EgShapedrawList_CollectText(ecs_iter_t *it)
{
	EgShapedrawList   *l0   = ecs_field_shared(it, EgShapedrawList, 0);
	EgBaseText const  *t   = ecs_field_self(it, EgBaseText, 1);
	EgBaseFont const  *f   = ecs_field_self(it, EgBaseFont, 2);
	EgBaseColor const *c = ecs_field_self(it, EgBaseColor, 3);
	for (int i = 0; i < it->count; ++i, ++t, ++f, ++c) {

	}
}

void EgShapedrawImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgShapedraw);
	ecs_set_name_prefix(world, "EgShapedraw");

	ECS_IMPORT(world, EgBase);
	ECS_IMPORT(world, EgShapes);
	ECS_IMPORT(world, EgSpatials);

	ECS_COMPONENT_DEFINE(world, EgShapedrawList);

	ecs_set_hooks(world, EgShapedrawList,
	{
	.ctor = EgShapedrawList_ctor,
	.dtor = EgShapedrawList_dtor,
	.move = EgShapedrawList_move,
	});

	ecs_struct(world,
	{.entity = ecs_id(EgShapedrawList),
	.members = {
	{.name = "data", .type = ecs_id(ecs_uptr_t)},
	{.name = "count", .type = ecs_id(ecs_i32_t)},
	{.name = "capacity", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgShapedrawList_Reset"}),
	.phase       = EcsPreUpdate,
	.callback    = EgShapedrawList_Reset,
	.query.terms = {
	{.id = ecs_id(EgShapedrawList), .src.id = EcsSelf, .inout = EcsInOut},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgShapedrawRectangle_Collect3D"}),
	.phase       = EcsPostUpdate,
	.callback    = EgShapedrawRectangle_Collect3D,
	.query.terms = {
	{.id = ecs_id(EgShapedrawList), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsInOut},
	{.id = ecs_id(EgShapesRectangle), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(WorldTransform4), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(EgBaseColor), .src.id = EcsSelf, .inout = EcsIn, .oper = EcsOptional},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgShapedrawRectangle_Collect2D"}),
	.phase       = EcsPostUpdate,
	.callback    = EgShapedrawRectangle_Collect2D,
	.query.terms = {
	{.id = ecs_id(EgShapedrawList), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsInOut},
	{.id = ecs_id(EgShapesRectangle), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(WorldTransform3), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(EgBaseColor), .src.id = EcsSelf, .inout = EcsIn},
	}});

	ecs_system(world,
	{.entity     = ecs_entity(world, {.name = "EgShapedrawList_CollectText"}),
	.phase       = EcsPostUpdate,
	.callback    = EgShapedrawList_CollectText,
	.query.terms = {
	{.id = ecs_id(EgShapedrawList), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsInOut},
	{.id = ecs_id(EgBaseText), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(EgBaseFont), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(EgBaseColor), .src.id = EcsSelf, .inout = EcsIn, .oper = EcsOptional},
	}});
}
