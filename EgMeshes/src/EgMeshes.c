#include "EgMeshes.h"
#include <EgShapes.h>
#include <EgBase.h>
#include <EgSpatials.h>
#include <ecsx.h>
#include <stdio.h>
#include <egmath.h>

ECS_COMPONENT_DECLARE(EgMeshesVertexInfo);
ECS_COMPONENT_DECLARE(EgMeshesVertices);
ECS_COMPONENT_DECLARE(EgMeshesSection);

static void extract_vertex_info(ecs_world_t *world, ecs_entity_t src, EgMeshesVertexInfo *info)
{
	ecs_iter_t it = ecs_children(world, src);
	while (ecs_children_next(&it)) {
		for (int i = 0; i < it.count; i++) {
			ecs_entity_t child = it.entities[i];
			EcsMember const *m = ecs_get(world, child, EcsMember);
			if (m == NULL) {
				ecs_err("m is NULL");
				break;
			}
			if (m->unit == EcsMeters) {
				// printf("m: %i\n", m->count);
				info->offset_pos = m->offset;
			}
			if (m->unit == EcsColorRgb) {
				// printf("m: %i\n", m->count);
				info->offset_col = m->offset;
			}
		}
	}
}

static void gen6_rectangle(EgMeshesVertices *vertices, EgMeshesVertexInfo *info)
{
	uint8_t *v = ecs_vec_grow(NULL, &vertices->vertices, info->size, 6);
	v4f32_xyzw((float *)(v + info->offset_col), 1.0f, 0.0f, 0.0f, 1.0f);
	v3f32_xyz((float *)(v + info->offset_pos), -0.5f, -0.5f, 0.0f);
	v += info->size;
	v4f32_xyzw((float *)(v + info->offset_col), 1.0f, 0.0f, 0.0f, 1.0f);
	v3f32_xyz((float *)(v + info->offset_pos), -0.5f, 0.5f, 0.0f);
	v += info->size;
	v4f32_xyzw((float *)(v + info->offset_col), 1.0f, 0.0f, 0.0f, 1.0f);
	v3f32_xyz((float *)(v + info->offset_pos), 0.5f, -0.5f, 0.0f);
	v += info->size;
	v4f32_xyzw((float *)(v + info->offset_col), 1.0f, 0.0f, 0.0f, 1.0f);
	v3f32_xyz((float *)(v + info->offset_pos), 0.5f, -0.5f, 0.0f);
	v += info->size;
	v4f32_xyzw((float *)(v + info->offset_col), 1.0f, 0.0f, 0.0f, 1.0f);
	v3f32_xyz((float *)(v + info->offset_pos), -0.5f, 0.5f, 0.0f);
	v += info->size;
	v4f32_xyzw((float *)(v + info->offset_col), 1.0f, 0.0f, 0.0f, 1.0f);
	v3f32_xyz((float *)(v + info->offset_pos), 0.5f, 0.5f, 0.0f);
	uint16_t *i = ecs_vec_grow_t(NULL, &vertices->indices, uint16_t, 6);
	vertices->index_counter += 6;
	i[0] = vertices->index_counter + 0;
	i[1] = vertices->index_counter + 1;
	i[2] = vertices->index_counter + 2;
	i[3] = vertices->index_counter + 3;
	i[4] = vertices->index_counter + 4;
	i[5] = vertices->index_counter + 5;
}

static void System_extract_vertex_info(ecs_iter_t *it)
{
	ecs_log_set_level(0);
	ecs_world_t *world = it->world;
	EcsComponent *c = ecs_field_self(it, EcsComponent, 0);
	EgMeshesVertexInfo *m = ecs_field_self(it, EgMeshesVertexInfo, 1);
	ecs_assert(c != NULL, ECS_INTERNAL_ERROR, NULL);
	for (int i = 0; i < it->count; ++i, ++m, ++c) {
		ecs_entity_t e = it->entities[i];
		m->size = c->size;
		m->alignment = c->alignment;
		extract_vertex_info(world, e, m);
	} // END FOR LOOP
	ecs_log_set_level(-1);
}

static void System_gen_verts(ecs_iter_t *it)
{
	ecs_log_set_level(0);
	ecs_world_t *world = it->world;
	EgShapesRectangle *c = ecs_field_self(it, EgShapesRectangle, 0);
	EgMeshesVertices *v = ecs_field_shared(it, EgMeshesVertices, 1);
	for (int i = 0; i < it->count; ++i) {
		ecs_entity_t e = it->entities[i];
		// Print entity name
		printf("Generating verts for entity: %s\n", ecs_get_name(world, e));
		// gen6_rectangle(v, NULL);
		ecs_set(world, e, EgMeshesSection, {.offset = v->index_counter, .count = 6});
	} // END FOR LOOP
	ecs_log_set_level(-1);
}

ECS_CTOR(EgMeshesVertices, ptr, {
	ecs_vec_init(NULL, &ptr->vertices, sizeof(uint8_t), 0);
	ecs_vec_init(NULL, &ptr->indices, sizeof(uint16_t), 0);
})

// The destructor should free resources.
ECS_DTOR(EgMeshesVertices, ptr, {
	ecs_vec_fini(NULL, &ptr->vertices, sizeof(uint8_t));
	ecs_vec_fini(NULL, &ptr->indices, sizeof(uint16_t));
});

// The move hook should move resources from one location to another.
ECS_MOVE(EgMeshesVertices, dst, src, {
	ecs_vec_fini(NULL, &dst->vertices, sizeof(uint8_t));
	ecs_vec_fini(NULL, &dst->indices, sizeof(uint16_t));
	*dst = *src;
	src->vertices.array = NULL;
	src->vertices.count = 0;
	src->vertices.size = 0;
	src->indices.array = NULL;
	src->indices.count = 0;
	src->indices.size = 0;
});

// The copy hook should copy resources from one location to another.
ECS_COPY(EgMeshesVertices, dst, src, {
	ecs_vec_fini(NULL, &dst->vertices, sizeof(uint8_t));
	ecs_vec_fini(NULL, &dst->indices, sizeof(uint16_t));
	dst->vertices = ecs_vec_copy(NULL, &src->vertices, sizeof(uint8_t));
	dst->indices = ecs_vec_copy(NULL, &src->indices, sizeof(uint16_t));
})

void EgMeshesImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgMeshes);
	ecs_set_name_prefix(world, "EgMeshes");
	ECS_IMPORT(world, EgShapes);
	ECS_IMPORT(world, EgSpatials);
	ECS_IMPORT(world, EgBase);

	ECS_COMPONENT_DEFINE(world, EgMeshesVertexInfo);
	ECS_COMPONENT_DEFINE(world, EgMeshesVertices);
	ECS_COMPONENT_DEFINE(world, EgMeshesSection);

	ecs_set_hooks(world, EgMeshesVertices,
	{
	.ctor = ecs_ctor(EgMeshesVertices),
	.move = ecs_move(EgMeshesVertices),
	.copy = ecs_copy(EgMeshesVertices),
	.dtor = ecs_dtor(EgMeshesVertices),
	});

	ecs_struct(world,
	{.entity = ecs_id(EgMeshesVertexInfo),
	.members = {
	{.name = "size", .type = ecs_id(ecs_i32_t)},
	{.name = "alignment", .type = ecs_id(ecs_i32_t)},
	{.name = "offset_pos", .type = ecs_id(ecs_i32_t)},
	{.name = "offset_col", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgMeshesVertices),
	.members = {
	{.name = "vertices", .type = ecs_id(EgBaseVec)},
	{.name = "indices", .type = ecs_id(EgBaseVec)},
	{.name = "index_counter", .type = ecs_id(ecs_u32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgMeshesSection),
	.members = {
	{.name = "offset", .type = ecs_id(ecs_u32_t)},
	{.name = "count", .type = ecs_id(ecs_u32_t)},
	}});

	ecs_observer_init(world,
	&(ecs_observer_desc_t){
	.entity = ecs_entity(world, {.name = "System_extract_vertex_info", .add = ecs_ids(ecs_dependson(EcsOnLoad))}),
	.callback = System_extract_vertex_info,
	.events = {EcsOnSet},
	.yield_existing = true, // Ensure existing components are processed. This does not work yet.
	.query.terms = {
	{.id = ecs_id(EcsComponent), .inout = EcsIn},
	{.id = ecs_id(EgMeshesVertexInfo), .inout = EcsInOut},
	},
	});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "System_gen_verts", .add = ecs_ids(ecs_dependson(EcsOnValidate))}),
	.callback = System_gen_verts,
	.query.terms =
	{
	{.id = ecs_id(EgShapesRectangle), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(EgMeshesVertices), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsInOut},
	{.id = ecs_id(EgMeshesVertexInfo), .trav = EcsDependsOn, .src.id = EcsUp, .inout = EcsInOut},
	{.id = ecs_id(EgMeshesSection), .src.id = EcsSelf, .oper = EcsNot},
	}});
}