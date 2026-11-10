#include "EgDraws.h"
#include "verts.h"

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
	int32_t layerCount = ecs_vec_count(&l->layers);
	ecs_vec_t *layers = ecs_vec_first_t(&l->layers, ecs_vec_t);
	for (int32_t i = 0; i < layerCount; ++i) {
		ecs_vec_fini_t(NULL, &layers[i], EgDraws_Vertex);
	}
	ecs_vec_fini_t(NULL, &l->layers, ecs_vec_t);
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
		src[i].layers        = (ecs_vec_t){0};
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
	ecs_vec_t *layers = ecs_vec_first_t(&list->layers, ecs_vec_t);
	for (int32_t i = 0, count = ecs_vec_count(&list->layers); i < count; ++i) {
		ecs_vec_clear(&layers[i]);
	}
}

void EgDraws_List_SetPixelScale(EgDraws_List *list, float pixelScale)
{
	list->pixelScale = pixelScale > 0.0f ? pixelScale : 1.0f;
}

ecs_vec_t *EgDraws_List_GetLayer(EgDraws_List *list, int32_t z)
{
	if (z < 0) {
		z = 0;
	}

	int32_t layerCount = ecs_vec_count(&list->layers);
	if (z >= layerCount) {
		int32_t newLayerCount = z + 1 - layerCount;
		ecs_vec_t *newLayers = ecs_vec_grow_t(NULL, &list->layers, ecs_vec_t, newLayerCount);
		if (newLayers == NULL) {
			return NULL;
		}
		memset(newLayers, 0, (size_t)newLayerCount * sizeof(ecs_vec_t));
		for (int32_t i = 0; i < newLayerCount; ++i) {
			ecs_vec_init_t(NULL, &newLayers[i], EgDraws_Vertex, 0);
		}
	}

	return ecs_vec_get_t(&list->layers, ecs_vec_t, z);
}

void EgDraws_List_Append(EgDraws_List *dst, const EgDraws_List *src)
{
	const ecs_vec_t *srcLayers = ecs_vec_first_t(&src->layers, ecs_vec_t);
	for (int32_t z = 0, layerCount = ecs_vec_count(&src->layers); z < layerCount; ++z) {
		const ecs_vec_t *s = &srcLayers[z];
		int32_t srcCount = ecs_vec_count(s);
		if (srcCount == 0) {
			continue;
		}

		ecs_vec_t *dstVertices = EgDraws_List_GetLayer(dst, z);
		if (dstVertices == NULL) {
			return;
		}

		EgDraws_Vertex *vertices = ecs_vec_grow_t(NULL, dstVertices, EgDraws_Vertex, srcCount);
		if (vertices == NULL) {
			return;
		}

		memcpy(vertices, ecs_vec_first(s), (size_t)srcCount * sizeof(EgDraws_Vertex));
	}
}

static ecs_vec_t *EgDraws_BeginVertices(EgDraws_List *list, int32_t z, const m3f32 *transform, uint32_t color, verts_config_t *config)
{
	ecs_vec_t *vertices = EgDraws_List_GetLayer(list, z);
	if (vertices == NULL) {
		ecs_err("EgDraws: failed to allocate layer %d", z);
		return NULL;
	}

	*config = (verts_config_t){
		.pixscale = list->pixelScale,
		.transform = *transform,
		.r = (uint8_t)(color >> 16),
		.g = (uint8_t)(color >> 8),
		.b = (uint8_t)color,
		.a = (uint8_t)(color >> 24),
	};
	if (config->a == 0) {
		config->a = 255;
	}
	return vertices;
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
		if (r->w <= 0.0f || r->h <= 0.0f) {
			continue;
		}
		int32_t z = zs != NULL ? zs[i].z : 0;
		verts_config_t config;
		ecs_vec_t *vertices = EgDraws_BeginVertices(l0, z, &w->matrix, s->color, &config);
		if (vertices != NULL) {
			verts_add_rectangle(vertices, &config, r->w, r->h);
		}
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
		if (r->w <= 0.0f || r->h <= 0.0f || b->thickness <= 0.0f) {
			continue;
		}
		int32_t z = zs != NULL ? zs[i].z : 0;
		verts_config_t config;
		ecs_vec_t *vertices = EgDraws_BeginVertices(l0, z, &w->matrix, b->color, &config);
		if (vertices != NULL) {
			verts_add_rectangle_outline(vertices, &config, r->w, r->h, b->thickness);
		}
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
		int32_t z = z_optional != NULL ? z_optional[i].z : 0;
		if (t->value == NULL || t->value[0] == '\0' || EgDraws_Font_GetBitmap() == NULL) {
			continue;
		}
		float fontSize = f->font_size > 0.0f ? f->font_size : 24.0f;
		verts_config_t config;
		ecs_vec_t *vertices = EgDraws_BeginVertices(l0, z, &x->matrix, f->color, &config);
		if (vertices != NULL) {
			verts_add_text(vertices, &config, fontSize, t->value);
		}
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
