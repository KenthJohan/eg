#include "EgDraws.h"

#include <EgBase.h>
#include <EgShapes.h>
#include <EgSpatials.h>
#include <ecsx.h>
#include <stdlib.h>
#include <string.h>

ECS_COMPONENT_DECLARE(EgDraws_List);
ECS_COMPONENT_DECLARE(EgDraws_Z);
ECS_COMPONENT_DECLARE(EgDraws_Border);
ECS_COMPONENT_DECLARE(EgDraws_Solid);

static void EgDraws_List_Free(EgDraws_List *l)
{
	for (int32_t i = 0; i < l->layerCount; ++i) {
		free(l->layers[i].data);
	}
	free(l->layers);
	l->layers        = NULL;
	l->layerCount    = 0;
	l->layerCapacity = 0;
}

static void EgDraws_List_ctor(void *ptr, int32_t count, const ecs_type_info_t *ti)
{
	(void)ti;
	EgDraws_List *l = ptr;
	memset(ptr, 0, (size_t)count * sizeof(EgDraws_List));
	for (int32_t i = 0; i < count; ++i) {
		l[i].pixelScale = 1.0f;
	}
}

static void EgDraws_List_dtor(void *ptr, int32_t count, const ecs_type_info_t *ti)
{
	(void)ti;
	EgDraws_List *l = ptr;
	for (int32_t i = 0; i < count; ++i) {
		EgDraws_List_Free(&l[i]);
	}
}

static void EgDraws_List_move(void *dst_ptr, void *src_ptr, int32_t count, const ecs_type_info_t *ti)
{
	(void)ti;
	EgDraws_List *dst = dst_ptr;
	EgDraws_List *src = src_ptr;
	for (int32_t i = 0; i < count; ++i) {
		EgDraws_List_Free(&dst[i]);
		dst[i]               = src[i];
		src[i].layers        = NULL;
		src[i].layerCount    = 0;
		src[i].layerCapacity = 0;
	}
}

EgDraws_List *EgDraws_List_Create(void)
{
	EgDraws_List *l = calloc(1, sizeof(*l));
	if (l != NULL) {
		l->pixelScale = 1.0f;
	}
	return l;
}

void EgDraws_List_Destroy(EgDraws_List *list)
{
	if (list == NULL) {
		return;
	}
	EgDraws_List_Free(list);
	free(list);
}

void EgDraws_List_Clear(EgDraws_List *list)
{
	for (int32_t i = 0; i < list->layerCount; ++i) {
		list->layers[i].count = 0;
	}
}

void EgDraws_List_SetPixelScale(EgDraws_List *list, float pixelScale)
{
	list->pixelScale = pixelScale > 0.0f ? pixelScale : 1.0f;
}

EgDraws_Layer *EgDraws_List_GetLayer(EgDraws_List *list, int32_t z)
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
			EgDraws_Layer *layers = realloc(list->layers, (size_t)cap * sizeof(EgDraws_Layer));
			if (layers == NULL) {
				return NULL;
			}
			list->layers        = layers;
			list->layerCapacity = cap;
		}
		memset(list->layers + list->layerCount, 0, (size_t)(z + 1 - list->layerCount) * sizeof(EgDraws_Layer));
		list->layerCount = z + 1;
	}

	return &list->layers[z];
}

void EgDraws_List_Append(EgDraws_List *dst, const EgDraws_List *src)
{
	for (int32_t z = 0; z < src->layerCount; ++z) {
		const EgDraws_Layer *s = &src->layers[z];
		if (s->count == 0) {
			continue;
		}

		EgDraws_Layer *d = EgDraws_List_GetLayer(dst, z);
		if (d == NULL) {
			return;
		}

		if (d->count + s->count > d->capacity) {
			int32_t cap = d->capacity > 0 ? d->capacity : 256;
			while (cap < d->count + s->count) {
				cap *= 2;
			}
			EgDraws_Vertex *data = realloc(d->data, (size_t)cap * sizeof(EgDraws_Vertex));
			if (data == NULL) {
				return;
			}
			d->data     = data;
			d->capacity = cap;
		}

		memcpy(d->data + d->count, s->data, (size_t)s->count * sizeof(EgDraws_Vertex));
		d->count += s->count;
	}
}

static void EgDraws_AddRectangle(EgDraws_List *l, int32_t z, const EgShapesRectangle *r, const m3f32 *transform, uint32_t color)
{
	EgDraws_List_AddRectangle(l, z, transform, r->w, r->h, color);
}

static void EgDraws_List_Reset(ecs_iter_t *it)
{
	EgDraws_List *l = ecs_field_self(it, EgDraws_List, 0);
	for (int i = 0; i < it->count; ++i) {
		EgDraws_List_Clear(&l[i]);
	}
}

static void EgDraws_Solid_Collect2D(ecs_iter_t *it)
{
	EgDraws_List         *l0 = ecs_field_shared(it, EgDraws_List, 0);
	EgDraws_Solid const  *s  = ecs_field_self(it, EgDraws_Solid, 1);
	EgShapesRectangle const *r  = ecs_field_self(it, EgShapesRectangle, 2);
	WorldTransform3 const   *w  = ecs_field_self(it, WorldTransform3, 3);
	EgDraws_Z const      *zs = ecs_field_self(it, EgDraws_Z, 4); // Optional
	for (int i = 0; i < it->count; ++i, ++s, ++r, ++w) {
		float z = zs != NULL ? zs[i].z : 0;
		EgDraws_List_AddRectangle(l0, z, &w->matrix, r->w, r->h, s->color);
	}
}

static void EgDraws_Border_Collect2D(ecs_iter_t *it)
{
	EgDraws_List         *l0 = ecs_field_shared(it, EgDraws_List, 0);
	EgDraws_Border const *b  = ecs_field_self(it, EgDraws_Border, 1);
	EgShapesRectangle const *r  = ecs_field_self(it, EgShapesRectangle, 2);
	WorldTransform3 const   *w  = ecs_field_self(it, WorldTransform3, 3);
	EgDraws_Z const      *zs = ecs_field_self(it, EgDraws_Z, 4); // Optional
	for (int i = 0; i < it->count; ++i, ++b, ++r, ++w) {
		float z = zs != NULL ? zs[i].z : 0;
		EgDraws_List_AddRectangleOutline(l0, z, &w->matrix, r->w, r->h, b->thickness, b->color);
	}
}

static void EgDraws_List_CollectText(ecs_iter_t *it)
{
	EgDraws_List       *l0         = ecs_field_shared(it, EgDraws_List, 0);
	EgBaseText const      *t          = ecs_field_self(it, EgBaseText, 1);
	EgBaseFont const      *f          = ecs_field_self(it, EgBaseFont, 2);
	WorldTransform3 const *x          = ecs_field_self(it, WorldTransform3, 3);
	EgDraws_Z const    *z_optional = ecs_field_self(it, EgDraws_Z, 4);

	for (int i = 0; i < it->count; ++i, ++t, ++f, ++x) {
		float z = z_optional != NULL ? z_optional[i].z : 0;
		if (t->value == NULL || t->value[0] == '\0') {
			continue;
		}
		float fontSize = f->font_size > 0.0f ? f->font_size : 24.0f;
		EgDraws_List_AddText(l0, z, &(x->matrix), fontSize, f->color, t->value);
	}
}

void EgDraws_Import(ecs_world_t *world)
{
	ECS_MODULE(world, EgDraws);
	ecs_set_name_prefix(world, "EgDraws_");

	ECS_IMPORT(world, EgBase);
	ECS_IMPORT(world, EgShapes);
	ECS_IMPORT(world, EgSpatials);

	ECS_COMPONENT_DEFINE(world, EgDraws_List);
	ECS_COMPONENT_DEFINE(world, EgDraws_Z);
	ECS_COMPONENT_DEFINE(world, EgDraws_Border);
	ECS_COMPONENT_DEFINE(world, EgDraws_Solid);

	ecs_set_hooks(world, EgDraws_List,
	{
	.ctor = EgDraws_List_ctor,
	.dtor = EgDraws_List_dtor,
	.move = EgDraws_List_move,
	});

	ecs_struct(world,
	{.entity = ecs_id(EgDraws_List),
	.members = {
	{.name = "layers", .type = ecs_id(ecs_uptr_t)},
	{.name = "layerCount", .type = ecs_id(ecs_i32_t)},
	{.name = "layerCapacity", .type = ecs_id(ecs_i32_t)},
	{.name = "pixelScale", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgDraws_Border),
	.members = {
	{.name = "thickness", .type = ecs_id(ecs_f32_t)},
	{.name = "color", .type = ecs_id(ecs_u32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgDraws_Solid),
	.members = {
	{.name = "color", .type = ecs_id(ecs_u32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgDraws_Z),
	.members = {
	{.name = "z", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgDraws_List_Reset"}),
	.phase       = EcsPreUpdate,
	.callback    = EgDraws_List_Reset,
	.query.terms = {
	{.id = ecs_id(EgDraws_List), .src.id = EcsSelf, .inout = EcsInOut},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgDraws_Solid_Collect2D"}),
	.phase       = EcsPostUpdate,
	.callback    = EgDraws_Solid_Collect2D,
	.query.terms = {
	{.id = ecs_id(EgDraws_List), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsInOut},
	{.id = ecs_id(EgDraws_Solid), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(EgShapesRectangle), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(WorldTransform3), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(EgDraws_Z), .src.id = EcsSelf, .inout = EcsIn, .oper = EcsOptional},
	{.id = EcsDisabled, .trav = EcsChildOf, .src.id = EcsUp, .oper = EcsNot}}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgDraws_Border_Collect2D"}),
	.phase       = EcsPostUpdate,
	.callback    = EgDraws_Border_Collect2D,
	.query.terms = {
	{.id = ecs_id(EgDraws_List), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsInOut},
	{.id = ecs_id(EgDraws_Border), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(EgShapesRectangle), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(WorldTransform3), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(EgDraws_Z), .src.id = EcsSelf, .inout = EcsIn, .oper = EcsOptional},
	{.id = EcsDisabled, .trav = EcsChildOf, .src.id = EcsUp, .oper = EcsNot},
	}});

	ecs_system(world,
	{.entity     = ecs_entity(world, {.name = "EgDraws_List_CollectText"}),
	.phase       = EcsPostUpdate,
	.callback    = EgDraws_List_CollectText,
	.query.terms = {
	{.id = ecs_id(EgDraws_List), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsInOut},
	{.id = ecs_id(EgBaseText), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(EgBaseFont), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(WorldTransform3), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(EgDraws_Z), .src.id = EcsSelf, .inout = EcsIn, .oper = EcsOptional},
	{.id = EcsDisabled, .trav = EcsChildOf, .src.id = EcsUp, .oper = EcsNot},
	}});
}
