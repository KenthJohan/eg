#include "EgDraws.h"
#include "verts.h"

#include <EgBase.h>
#include <EgCameras.h>
#include <EgShapes.h>
#include <EgSpatials.h>
#include <ecsx.h>
#include <stdlib.h>
#include <string.h>

ECS_COMPONENT_DECLARE(EgDraws_List);
ECS_COMPONENT_DECLARE(EgDraws_Z);
ECS_COMPONENT_DECLARE(EgDraws_Border);
ECS_COMPONENT_DECLARE(EgDraws_Solid);
ECS_TAG_DECLARE(EgDraws_Camera);
ECS_DECLARE(EgDraws_GeometryPhase);

static void EgDraws_List_Free(EgDraws_List *l)
{
	int32_t    layerCount = ecs_vec_count(&l->layers);
	ecs_vec_t *layers     = ecs_vec_first_t(&l->layers, ecs_vec_t);
	for (int32_t i = 0; i < layerCount; ++i) {
		ecs_vec_fini_t(NULL, &layers[i], EgDraws_Vertex);
	}
	ecs_vec_fini_t(NULL, &l->layers, ecs_vec_t);
	ecs_vec_fini_t(NULL, &l->spans, EgDraws_Span);
}

static void EgDraws_List_ctor(void *ptr, int32_t count, const ecs_type_info_t *ti)
{
	(void)ti;
	memset(ptr, 0, (size_t)count * sizeof(EgDraws_List));
	EgDraws_List *lists = ptr;
	for (int32_t i = 0; i < count; ++i) {
		lists[i].pixel_scale = 1.0f;
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
		dst[i]        = src[i];
		src[i].layers = (ecs_vec_t){0};
		src[i].spans  = (ecs_vec_t){0};
	}
}

EgDraws_List *EgDraws_List_Create(void)
{
	EgDraws_List *list = calloc(1, sizeof(EgDraws_List));
	if (list != NULL) {
		list->pixel_scale = 1.0f;
	}
	return list;
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
	ecs_vec_clear(&list->spans);
	list->failed = false;
}

ecs_vec_t *EgDraws_List_GetLayer(EgDraws_List *list, int32_t z)
{
	if (z < 0) {
		z = 0;
	}

	int32_t layerCount = ecs_vec_count(&list->layers);
	if (z >= layerCount) {
		int32_t    newLayerCount = z + 1 - layerCount;
		ecs_vec_t *newLayers     = ecs_vec_grow_t(NULL, &list->layers, ecs_vec_t, newLayerCount);
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

bool EgDraws_List_Append(EgDraws_List *dst, const EgDraws_List *src)
{
	if (dst == src) {
		ecs_err("EgDraws: cannot append a list to itself");
		dst->failed = true;
		return false;
	}
	if (src->failed || dst->failed) {
		dst->failed = true;
		ecs_err("EgDraws: cannot append an incomplete list");
		return false;
	}
	const ecs_vec_t *srcLayers = ecs_vec_first_t(&src->layers, ecs_vec_t);
	for (int32_t z = 0, layerCount = ecs_vec_count(&src->layers); z < layerCount; ++z) {
		const ecs_vec_t *s        = &srcLayers[z];
		int32_t          srcCount = ecs_vec_count(s);
		if (srcCount == 0) {
			continue;
		}

		ecs_vec_t *dstVertices = EgDraws_List_GetLayer(dst, z);
		if (dstVertices == NULL) {
			dst->failed = true;
			ecs_err("EgDraws: failed to allocate destination layer");
			return false;
		}

		int32_t first = ecs_vec_count(dstVertices);
		EgDraws_Vertex *vertices = ecs_vec_grow_t(NULL, dstVertices, EgDraws_Vertex, srcCount);
		if (vertices == NULL) {
			dst->failed = true;
			ecs_err("EgDraws: failed to append vertices");
			return false;
		}

		memcpy(vertices, ecs_vec_first(s), (size_t)srcCount * sizeof(EgDraws_Vertex));
		const EgDraws_Span *spans = ecs_vec_first_t(&src->spans, EgDraws_Span);
		for (int32_t i = 0; i < ecs_vec_count(&src->spans); ++i) {
			if (spans[i].layer != z)
				continue;
			EgDraws_Span *copy = ecs_vec_append_t(NULL, &dst->spans, EgDraws_Span);
			if (!copy) {
				dst->failed = true;
				ecs_err("EgDraws: failed to append draw span");
				return false;
			}
			*copy = spans[i];
			copy->first += first;
		}
	}
	return true;
}

static ecs_vec_t *EgDraws_BeginVertices(EgDraws_List *list, int32_t z, const m3f32 *transform, uint32_t color, verts_config_t *config)
{
	ecs_vec_t *vertices = EgDraws_List_GetLayer(list, z);
	if (vertices == NULL) {
		ecs_err("EgDraws: failed to allocate layer %d", z);
		list->failed = true;
		return NULL;
	}

	*config = (verts_config_t){
	.pixscale  = list->pixel_scale,
	.transform = *transform,
	.r         = (uint8_t)(color >> 16),
	.g         = (uint8_t)(color >> 8),
	.b         = (uint8_t)color,
	.a         = (uint8_t)(color >> 24),
	.failed    = &list->failed,
	};
	if (config->a == 0) {
		config->a = 255;
	}
	return vertices;
}

static void EgDraws_RecordSpan(EgDraws_List *list, ecs_vec_t *vertices,
int32_t z, int32_t first, ecs_entity_t entity, EgDraws_PrimitiveKind kind)
{
	int32_t count = ecs_vec_count(vertices) - first;
	if (!count || list->failed)
		return;
	EgDraws_Span *span = ecs_vec_append_t(NULL, &list->spans, EgDraws_Span);
	if (!span) {
		list->failed = true;
		ecs_err("EgDraws: failed to record draw span");
		return;
	}
	*span = (EgDraws_Span){entity, z < 0 ? 0 : z, first, count, kind};
}

static void EgDraws_List_Reset(ecs_iter_t *it)
{
	EgDraws_List *l = ecs_field_self(it, EgDraws_List, 0);
	for (int i = 0; i < it->count; ++i) {
		EgDraws_List_Clear(&l[i]);
	}
}

static void EgDraws_List_UpdatePixelScale(ecs_iter_t *it)
{
	EgDraws_List *lists = ecs_field_self(it, EgDraws_List, 0);
	for (int32_t i = 0; i < it->count; ++i) {
		ecs_entity_t camera_entity = ecs_get_target(
		it->world, it->entities[i], EgDraws_Camera, 0);
		const EgCamerasState *camera = camera_entity ?
		ecs_get(it->world, camera_entity, EgCamerasState) : NULL;
		if (!camera || ecs_get_target(it->world, it->entities[i], EgDraws_Camera, 1)) {
			ecs_err("EgDraws: draw list requires one explicit camera");
			lists[i].failed = true;
			continue;
		}
		lists[i].pixel_scale = camera->pixel_scale;
	}
}

static void EgDraws_Solid_Collect2D(ecs_iter_t *it)
{
	EgDraws_List            *l0 = ecs_field_shared(it, EgDraws_List, 0);
	EgDraws_Solid const     *s  = ecs_field_self(it, EgDraws_Solid, 1);
	EgShapesRectangle const *r  = ecs_field_self(it, EgShapesRectangle, 2);
	WorldTransform3 const   *w  = ecs_field_self(it, WorldTransform3, 3);
	EgDraws_Z const         *zs = ecs_field_self(it, EgDraws_Z, 4); // Optional
	for (int i = 0; i < it->count; ++i, ++s, ++r, ++w) {
		if (r->w <= 0.0f || r->h <= 0.0f) {
			continue;
		}
		int32_t        z = zs != NULL ? zs[i].z : 0;
		verts_config_t config;
		ecs_vec_t     *vertices = EgDraws_BeginVertices(l0, z, &w->matrix, s->color, &config);
		if (vertices != NULL) {
			int32_t first = ecs_vec_count(vertices);
			config.pick_id = (uint32_t)it->entities[i];
			verts_add_rectangle(vertices, &config, r->w, r->h);
			EgDraws_RecordSpan(l0, vertices, z, first, it->entities[i], EgDrawsPrimitiveSolid);
		}
	}
}

static void EgDraws_Border_Collect2D(ecs_iter_t *it)
{
	EgDraws_List            *l0 = ecs_field_shared(it, EgDraws_List, 0);
	EgDraws_Border const    *b  = ecs_field_self(it, EgDraws_Border, 1);
	EgShapesRectangle const *r  = ecs_field_self(it, EgShapesRectangle, 2);
	WorldTransform3 const   *w  = ecs_field_self(it, WorldTransform3, 3);
	EgDraws_Z const         *zs = ecs_field_self(it, EgDraws_Z, 4); // Optional
	for (int i = 0; i < it->count; ++i, ++b, ++r, ++w) {
		if (r->w <= 0.0f || r->h <= 0.0f || b->thickness <= 0.0f) {
			continue;
		}
		int32_t        z = zs != NULL ? zs[i].z : 0;
		verts_config_t config;
		ecs_vec_t     *vertices = EgDraws_BeginVertices(l0, z, &w->matrix, b->color, &config);
		if (vertices != NULL) {
			int32_t first = ecs_vec_count(vertices);
			config.pick_id = (uint32_t)it->entities[i];
			verts_add_rectangle_outline(vertices, &config, r->w, r->h, b->thickness);
			EgDraws_RecordSpan(l0, vertices, z, first, it->entities[i], EgDrawsPrimitiveBorder);
		}
	}
}

static void EgDraws_List_CollectText(ecs_iter_t *it)
{
	EgDraws_List          *l0         = ecs_field_shared(it, EgDraws_List, 0);
	EgBaseText const      *t          = ecs_field_self(it, EgBaseText, 1);
	EgBaseFont const      *f          = ecs_field_self(it, EgBaseFont, 2);
	WorldTransform3 const *x          = ecs_field_self(it, WorldTransform3, 3);
	EgDraws_Z const       *z_optional = ecs_field_self(it, EgDraws_Z, 4);

	for (int i = 0; i < it->count; ++i, ++t, ++f, ++x) {
		int32_t z = z_optional != NULL ? z_optional[i].z : 0;
		if (t->value == NULL || t->value[0] == '\0' || EgDraws_Font_GetBitmap() == NULL) {
			continue;
		}
		float          fontSize = f->font_size > 0.0f ? f->font_size : 24.0f;
		verts_config_t config;
		ecs_vec_t     *vertices = EgDraws_BeginVertices(l0, z, &x->matrix, f->color, &config);
		if (vertices != NULL) {
			int32_t first = ecs_vec_count(vertices);
			config.pick_id = (uint32_t)it->entities[i];
			verts_add_text(vertices, &config, fontSize, t->value);
			EgDraws_RecordSpan(l0, vertices, z, first, it->entities[i], EgDrawsPrimitiveText);
		}
	}
}

void EgDraws_Import(ecs_world_t *world)
{
	ECS_MODULE(world, EgDraws);
	ecs_set_name_prefix(world, "EgDraws_");

	ECS_IMPORT(world, EgBase);
	ECS_IMPORT(world, EgCameras);
	ECS_IMPORT(world, EgShapes);
	ECS_IMPORT(world, EgSpatials);

	ECS_COMPONENT_DEFINE(world, EgDraws_List);
	ECS_COMPONENT_DEFINE(world, EgDraws_Z);
	ECS_COMPONENT_DEFINE(world, EgDraws_Border);
	ECS_COMPONENT_DEFINE(world, EgDraws_Solid);
	ECS_TAG_DEFINE(world, EgDraws_Camera);
	ecs_entity_t pixel_phase = ecs_entity(world, {.name = "PixelScalePhase"});
	ecs_add_id(world, pixel_phase, EcsPhase);
	ecs_add_pair(world, pixel_phase, EcsDependsOn, EcsPostUpdate);
	EgDraws_GeometryPhase = ecs_entity(world, {.name = "GeometryPhase"});
	ecs_add_id(world, EgDraws_GeometryPhase, EcsPhase);
	ecs_add_pair(world, EgDraws_GeometryPhase, EcsDependsOn, pixel_phase);
	ecs_add_pair(world, EcsPreStore, EcsDependsOn, EgDraws_GeometryPhase);

	ecs_set_hooks(world, EgDraws_List,
	{
	.ctor = EgDraws_List_ctor,
	.dtor = EgDraws_List_dtor,
	.move = EgDraws_List_move,
	});

	ECS_COMPONENT(world, ecs_vec_t);

	ecs_struct(world,
	{.entity = ecs_id(ecs_vec_t),
	.members = {
	{.name = "element_size", .type = ecs_id(ecs_uptr_t)},
	{.name = "count", .type = ecs_id(ecs_i32_t)},
	{.name = "capacity", .type = ecs_id(ecs_i32_t)},
#ifdef FLECS_SANITIZE
	{.name = "elem_size", .type = ecs_id(ecs_size_t)},
	{.name = "type_name", .type = ecs_id(ecs_string_t)},
#endif
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgDraws_List),
	.members = {
	{.name = "layers", .type = ecs_id(ecs_vec_t)},
	{.name = "spans", .type = ecs_id(ecs_vec_t)},
	{.name = "pixel_scale", .type = ecs_id(ecs_f32_t)},
	{.name = "failed", .type = ecs_id(ecs_bool_t)},
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
	.entity      = ecs_entity(world, {.name = "EgDraws_List_UpdatePixelScale"}),
	.phase       = pixel_phase,
	.callback    = EgDraws_List_UpdatePixelScale,
	.query.terms = {
	{.id = ecs_id(EgDraws_List), .src.id = EcsSelf, .inout = EcsInOut},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgDraws_Solid_Collect2D"}),
	.phase       = EgDraws_GeometryPhase,
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
	.phase       = EgDraws_GeometryPhase,
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
	.phase       = EgDraws_GeometryPhase,
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
