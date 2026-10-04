#include "EgShapedraw.h"

#include <EgBase.h>
#include <EgShapes.h>
#include <EgSpatials.h>
#include <ecsx.h>
#include <stdlib.h>
#include <string.h>

ECS_COMPONENT_DECLARE(EgShapedrawList);
ECS_COMPONENT_DECLARE(EgShapedrawZ);

static void EgShapedrawList_Free(EgShapedrawList *l)
{
	for (int32_t i = 0; i < l->layerCount; ++i) {
		free(l->layers[i].data);
	}
	free(l->layers);
	l->layers        = NULL;
	l->layerCount    = 0;
	l->layerCapacity = 0;
}

static void EgShapedrawList_ctor(void *ptr, int32_t count, const ecs_type_info_t *ti)
{
	(void)ti;
	EgShapedrawList *l = ptr;
	memset(ptr, 0, (size_t)count * sizeof(EgShapedrawList));
	for (int32_t i = 0; i < count; ++i) {
		l[i].pixelScale = 1.0f;
	}
}

static void EgShapedrawList_dtor(void *ptr, int32_t count, const ecs_type_info_t *ti)
{
	(void)ti;
	EgShapedrawList *l = ptr;
	for (int32_t i = 0; i < count; ++i) {
		EgShapedrawList_Free(&l[i]);
	}
}

static void EgShapedrawList_move(void *dst_ptr, void *src_ptr, int32_t count, const ecs_type_info_t *ti)
{
	(void)ti;
	EgShapedrawList *dst = dst_ptr;
	EgShapedrawList *src = src_ptr;
	for (int32_t i = 0; i < count; ++i) {
		EgShapedrawList_Free(&dst[i]);
		dst[i]               = src[i];
		src[i].layers        = NULL;
		src[i].layerCount    = 0;
		src[i].layerCapacity = 0;
	}
}

EgShapedrawList *EgShapedrawList_Create(void)
{
	EgShapedrawList *l = calloc(1, sizeof(*l));
	if (l != NULL) {
		l->pixelScale = 1.0f;
	}
	return l;
}

void EgShapedrawList_Destroy(EgShapedrawList *list)
{
	if (list == NULL) {
		return;
	}
	EgShapedrawList_Free(list);
	free(list);
}

void EgShapedrawList_Clear(EgShapedrawList *list)
{
	for (int32_t i = 0; i < list->layerCount; ++i) {
		list->layers[i].count = 0;
	}
}

void EgShapedrawList_SetPixelScale(EgShapedrawList *list, float pixelScale)
{
	list->pixelScale = pixelScale > 0.0f ? pixelScale : 1.0f;
}

EgShapedrawLayer *EgShapedrawList_GetLayer(EgShapedrawList *list, int32_t z)
{
	if (z < 0) {
		z = 0;
	}

	if (z >= list->layerCount) {
		if (z >= list->layerCapacity) {
			int32_t cap = list->layerCapacity > 0 ? list->layerCapacity : 4;
			while (cap <= z) {
				cap *= 2;
			}
			EgShapedrawLayer *layers = realloc(list->layers, (size_t)cap * sizeof(EgShapedrawLayer));
			if (layers == NULL) {
				return NULL;
			}
			list->layers        = layers;
			list->layerCapacity = cap;
		}
		memset(list->layers + list->layerCount, 0, (size_t)(z + 1 - list->layerCount) * sizeof(EgShapedrawLayer));
		list->layerCount = z + 1;
	}

	return &list->layers[z];
}

void EgShapedrawList_Append(EgShapedrawList *dst, const EgShapedrawList *src)
{
	for (int32_t z = 0; z < src->layerCount; ++z) {
		const EgShapedrawLayer *s = &src->layers[z];
		if (s->count == 0) {
			continue;
		}

		EgShapedrawLayer *d = EgShapedrawList_GetLayer(dst, z);
		if (d == NULL) {
			return;
		}

		if (d->count + s->count > d->capacity) {
			int32_t cap = d->capacity > 0 ? d->capacity : 256;
			while (cap < d->count + s->count) {
				cap *= 2;
			}
			EgShapedrawVertex *data = realloc(d->data, (size_t)cap * sizeof(EgShapedrawVertex));
			if (data == NULL) {
				return;
			}
			d->data     = data;
			d->capacity = cap;
		}

		memcpy(d->data + d->count, s->data, (size_t)s->count * sizeof(EgShapedrawVertex));
		d->count += s->count;
	}
}

// Centered rectangle; (a,b) and (c,d) are the transformed x and y axes, (tx,ty) the origin.
static void EgShapedraw_AddRectangle(EgShapedrawList *l, int32_t z, const EgShapesRectangle *r, float a, float b, float c, float d, float tx, float ty, uint32_t color)
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

	EgShapedrawList_AddTriangle(l, z, x[0], y[0], x[1], y[1], x[2], y[2], color);
	EgShapedrawList_AddTriangle(l, z, x[0], y[0], x[2], y[2], x[3], y[3], color);
}

static void EgShapedrawList_Reset(ecs_iter_t *it)
{
	EgShapedrawList *l = ecs_field_self(it, EgShapedrawList, 0);
	for (int i = 0; i < it->count; ++i) {
		EgShapedrawList_Clear(&l[i]);
	}
}

static void EgShapedrawRectangle_Collect3D(ecs_iter_t *it)
{
	EgShapedrawList         *l   = ecs_field_shared(it, EgShapedrawList, 0);
	EgShapesRectangle const *r   = ecs_field_self(it, EgShapesRectangle, 1);
	WorldTransform4 const   *w   = ecs_field_self(it, WorldTransform4, 2);
	EgBaseColor const       *col = ecs_field_self(it, EgBaseColor, 3);
	EgShapedrawZ const      *zs  = ecs_field_self(it, EgShapedrawZ, 4);
	for (int i = 0; i < it->count; ++i) {
		uint32_t color = col != NULL ? col[i].color : 0x00FFFF00u;
		EgShapedraw_AddRectangle(l, zs != NULL ? zs[i].z : 0, &r[i], w[i].matrix.c0[0], w[i].matrix.c0[1], w[i].matrix.c1[0], w[i].matrix.c1[1], w[i].matrix.c3[0], w[i].matrix.c3[1], color);
	}
}

static void EgShapedrawRectangle_Collect2D(ecs_iter_t *it)
{
	EgShapedrawList         *l   = ecs_field_shared(it, EgShapedrawList, 0);
	EgShapesRectangle const *r   = ecs_field_self(it, EgShapesRectangle, 1);
	WorldTransform3 const   *w   = ecs_field_self(it, WorldTransform3, 2);
	EgBaseColor const       *col = ecs_field_self(it, EgBaseColor, 3);
	EgShapedrawZ const      *zs  = ecs_field_self(it, EgShapedrawZ, 4);
	for (int i = 0; i < it->count; ++i) {
		uint32_t color = col != NULL ? col[i].color : 0x00FFFF00u;
		EgShapedraw_AddRectangle(l, zs != NULL ? zs[i].z : 0, &r[i], w[i].matrix.c0[0], w[i].matrix.c0[1], w[i].matrix.c1[0], w[i].matrix.c1[1], w[i].matrix.c2[0], w[i].matrix.c2[1], color);
	}
}

static void EgShapedrawList_CollectText(ecs_iter_t *it)
{
	EgShapedrawList       *l0         = ecs_field_shared(it, EgShapedrawList, 0);
	EgBaseText const      *t          = ecs_field_self(it, EgBaseText, 1);
	EgBaseFont const      *f          = ecs_field_self(it, EgBaseFont, 2);
	WorldTransform3 const *x          = ecs_field_self(it, WorldTransform3, 3);
	EgShapedrawZ const    *z_optional = ecs_field_self(it, EgShapedrawZ, 4);

	for (int i = 0; i < it->count; ++i, ++t, ++f, ++x) {
		float z = z_optional != NULL ? z_optional[i].z : 0;
		if (t->value == NULL || t->value[0] == '\0') {
			continue;
		}
		float fontSize = f->font_size > 0.0f ? f->font_size : 24.0f;
		EgShapedrawList_AddText(l0, z, &(x->matrix), fontSize, f->color, t->value);
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
	ECS_COMPONENT_DEFINE(world, EgShapedrawZ);

	ecs_set_hooks(world, EgShapedrawList,
	{
	.ctor = EgShapedrawList_ctor,
	.dtor = EgShapedrawList_dtor,
	.move = EgShapedrawList_move,
	});

	ecs_struct(world,
	{.entity = ecs_id(EgShapedrawList),
	.members = {
	{.name = "layers", .type = ecs_id(ecs_uptr_t)},
	{.name = "layerCount", .type = ecs_id(ecs_i32_t)},
	{.name = "layerCapacity", .type = ecs_id(ecs_i32_t)},
	{.name = "pixelScale", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgShapedrawZ),
	.members = {
	{.name = "z", .type = ecs_id(ecs_i32_t)},
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
	{.id = ecs_id(EgShapedrawZ), .src.id = EcsSelf, .inout = EcsIn, .oper = EcsOptional},
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
	{.id = ecs_id(EgShapedrawZ), .src.id = EcsSelf, .inout = EcsIn, .oper = EcsOptional},
	}});

	ecs_system(world,
	{.entity     = ecs_entity(world, {.name = "EgShapedrawList_CollectText"}),
	.phase       = EcsPostUpdate,
	.callback    = EgShapedrawList_CollectText,
	.query.terms = {
	{.id = ecs_id(EgShapedrawList), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsInOut},
	{.id = ecs_id(EgBaseText), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(EgBaseFont), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(WorldTransform3), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(EgShapedrawZ), .src.id = EcsSelf, .inout = EcsIn, .oper = EcsOptional},
	}});
}
