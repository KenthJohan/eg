#include "EgUi.h"

#include <EgShapes.h>
#include <EgPhysics.h>
#include <EgButtons.h>
#include <ecsx.h>
#include <math.h>
#include <string.h>

ECS_COMPONENT_DECLARE(EgUiFlow);
ECS_COMPONENT_DECLARE(EgUiDirection);
ECS_COMPONENT_DECLARE(EgUiTable);
ECS_COMPONENT_DECLARE(EgUiCell);
ECS_COMPONENT_DECLARE(EgUiButton);

static void EgUiMouseHitTesting_Update(ecs_iter_t *it)
{
	Position2               *p0 = ecs_field_shared(it, Position2, 0);
	EgShapesRectangle const *r  = ecs_field_self(it, EgShapesRectangle, 1);
	WorldTransform3 const   *x  = ecs_field_self(it, WorldTransform3, 2);
	EgUiButton              *b  = ecs_field_self(it, EgUiButton, 3);
	EgButtonsState          *s  = ecs_field_shared(it, EgButtonsState, 4);

	for (int32_t i = 0; i < it->count; ++i, ++r, ++x, ++b) {

		char const *name = ecs_get_name(it->world, it->entities[i]);

		float dx  = p0->x - x->matrix.c2[0];
		float dy  = p0->y - x->matrix.c2[1];
		float det = x->matrix.c0[0] * x->matrix.c1[1] - x->matrix.c1[0] * x->matrix.c0[1];

		if (fabsf(det) < 1e-8f) {
			continue;
		}

		float local_x = (dx * x->matrix.c1[1] - dy * x->matrix.c1[0]) / det;
		float local_y = (dy * x->matrix.c0[0] - dx * x->matrix.c0[1]) / det;

		b->hovered = (fabsf(local_x) <= fabsf(r->w) * 0.5f) && (fabsf(local_y) <= fabsf(r->h) * 0.5f);
		b->held    = b->hovered && !!(s->mouse[0] & EG_BUTTONS_STATE_PRESSED);

		printf("Button %s hovered: %d, held: %d\n", name, b->hovered, b->held);

		ecs_modified_id(it->world, it->entities[i], ecs_id(EgUiButton));
	}
}

static float EgUiTable_MapGet(const ecs_map_t *map, int32_t key)
{
	ecs_map_val_t *val = ecs_map_get(map, (ecs_map_key_t)(uint32_t)key);
	if (!val) {
		return 0.0f;
	}
	float f;
	memcpy(&f, val, sizeof(f));
	return f;
}

static void EgUiTable_MapMax(ecs_map_t *map, int32_t key, float value)
{
	ecs_map_val_t *val = ecs_map_ensure(map, (ecs_map_key_t)(uint32_t)key);
	float          f   = 0.0f;
	memcpy(&f, val, sizeof(f));
	if (value > f) {
		*val = 0;
		memcpy(val, &value, sizeof(value));
	}
}

// Sum of sizes (plus gaps) of indices [0, index).
static float EgUiTable_Offset(const ecs_map_t *map, int32_t index, float gap)
{
	float sum = 0.0f;
	for (int32_t k = 0; k < index; ++k) {
		sum += EgUiTable_MapGet(map, k) + gap;
	}
	return sum;
}

static void EgUiTable_ctor(void *ptr, int32_t count, const ecs_type_info_t *ti)
{
	(void)ti;
	EgUiTable *t = ptr;
	for (int32_t i = 0; i < count; ++i) {
		memset(&t[i], 0, sizeof(EgUiTable));
		ecs_map_init(&t[i].rows_height, NULL);
		ecs_map_init(&t[i].cols_width, NULL);
	}
}

static void EgUiTable_dtor(void *ptr, int32_t count, const ecs_type_info_t *ti)
{
	(void)ti;
	EgUiTable *t = ptr;
	for (int32_t i = 0; i < count; ++i) {
		ecs_map_fini(&t[i].rows_height);
		ecs_map_fini(&t[i].cols_width);
	}
}

static void EgUiTable_move(void *dst_ptr, void *src_ptr, int32_t count, const ecs_type_info_t *ti)
{
	(void)ti;
	EgUiTable *dst = dst_ptr;
	EgUiTable *src = src_ptr;
	for (int32_t i = 0; i < count; ++i) {
		ecs_map_fini(&dst[i].rows_height);
		ecs_map_fini(&dst[i].cols_width);
		dst[i] = src[i];
		ecs_map_init(&src[i].rows_height, NULL);
		ecs_map_init(&src[i].cols_width, NULL);
	}
}

static void EgUiTable_copy(void *dst_ptr, const void *src_ptr, int32_t count, const ecs_type_info_t *ti)
{
	(void)ti;
	EgUiTable       *dst = dst_ptr;
	const EgUiTable *src = src_ptr;
	for (int32_t i = 0; i < count; ++i) {
		ecs_map_fini(&dst[i].rows_height);
		ecs_map_fini(&dst[i].cols_width);
		ecs_map_init(&dst[i].rows_height, NULL);
		ecs_map_init(&dst[i].cols_width, NULL);
		// ecs_map_copy leaves dst finalized (dangling buckets) when src is uninitialized.
		if (ecs_map_is_init(&src[i].rows_height)) {
			ecs_map_copy(&dst[i].rows_height, &src[i].rows_height);
		}
		if (ecs_map_is_init(&src[i].cols_width)) {
			ecs_map_copy(&dst[i].cols_width, &src[i].cols_width);
		}
		dst[i].total_space = src[i].total_space;
		dst[i].row_gap     = src[i].row_gap;
		dst[i].col_gap     = src[i].col_gap;
		dst[i].row_count   = src[i].row_count;
		dst[i].col_count   = src[i].col_count;
	}
}

static void EgUiTable_Reset(ecs_iter_t *it)
{
	EgUiTable *table = ecs_field_self(it, EgUiTable, 0);
	for (int i = 0; i < it->count; ++i) {
		ecs_map_clear(&table[i].rows_height);
		ecs_map_clear(&table[i].cols_width);
		table[i].row_count     = 0;
		table[i].col_count     = 0;
		table[i].total_space.w = 0.0f;
		table[i].total_space.h = 0.0f;
	}
}

static void EgUiTable_Measure(ecs_iter_t *it)
{
	EgUiTable         *table = ecs_field_shared(it, EgUiTable, 1);
	EgUiCell          *cells = ecs_field_self(it, EgUiCell, 2);
	EgShapesRectangle *rects = ecs_field_self(it, EgShapesRectangle, 3);
	for (int i = 0; i < it->count; ++i) {
		if (cells[i].row < 0 || cells[i].col < 0) {
			continue;
		}
		EgUiTable_MapMax(&table->rows_height, cells[i].row, rects[i].h);
		EgUiTable_MapMax(&table->cols_width, cells[i].col, rects[i].w);
		if (cells[i].row + 1 > table->row_count) {
			table->row_count = cells[i].row + 1;
		}
		if (cells[i].col + 1 > table->col_count) {
			table->col_count = cells[i].col + 1;
		}
	}
}

static void EgUiTable_Finalize(ecs_iter_t *it)
{
	EgUiTable *table = ecs_field_self(it, EgUiTable, 0);
	for (int i = 0; i < it->count; ++i) {
		float w                = EgUiTable_Offset(&table[i].cols_width, table[i].col_count, table[i].col_gap);
		float h                = EgUiTable_Offset(&table[i].rows_height, table[i].row_count, table[i].row_gap);
		table[i].total_space.w = table[i].col_count > 0 ? w - table[i].col_gap : 0.0f;
		table[i].total_space.h = table[i].row_count > 0 ? h - table[i].row_gap : 0.0f;
	}
}

static void EgUiTable_Layout(ecs_iter_t *it)
{
	EgUiTable         *table     = ecs_field_shared(it, EgUiTable, 1);
	EgUiCell          *cells     = ecs_field_self(it, EgUiCell, 2);
	EgShapesRectangle *rects     = ecs_field_self(it, EgShapesRectangle, 3);
	Position2         *positions = ecs_field_self(it, Position2, 4);
	for (int i = 0; i < it->count; ++i) {
		if (cells[i].row < 0 || cells[i].col < 0) {
			continue;
		}
		float col_w = EgUiTable_MapGet(&table->cols_width, cells[i].col);
		float row_h = EgUiTable_MapGet(&table->rows_height, cells[i].row);
		float left  = EgUiTable_Offset(&table->cols_width, cells[i].col, table->col_gap);
		float top   = EgUiTable_Offset(&table->rows_height, cells[i].row, table->row_gap);
		// Child is centered in its cell; table is centered on the parent origin, rows grow downward (+y is up).
		positions[i].x = -table->total_space.w * 0.5f + left + col_w * 0.5f;
		positions[i].y = table->total_space.h * 0.5f - top - row_h * 0.5f;
	}
}

static bool EgUiFlow_GetAxis(EgUiDirection direction, int *axis, float *sign)
{
	switch (direction) {
	case EgUiDirectionRight:
		*axis = 0;
		*sign = 1.0f;
		return true;
	case EgUiDirectionLeft:
		*axis = 0;
		*sign = -1.0f;
		return true;
	case EgUiDirectionUp:
		*axis = 1;
		*sign = 1.0f;
		return true;
	case EgUiDirectionDown:
		*axis = 1;
		*sign = -1.0f;
		return true;
	default:
		return false;
	}
}

static void EgUiFlow_Reset(ecs_iter_t *it)
{
	EgUiFlow          *flow   = ecs_field_self(it, EgUiFlow, 0);
	EgShapesRectangle *bounds = ecs_field_self(it, EgShapesRectangle, 1);
	for (int i = 0; i < it->count; ++i) {
		int   primary_axis = 0;
		float primary_sign = 0.0f;
		int   wrap_axis    = 0;
		float wrap_sign    = 0.0f;
		if (!EgUiFlow_GetAxis(flow[i].direction, &primary_axis, &primary_sign)) {
			flow[i].cursor_primary    = 0.0f;
			flow[i].cursor_wrap       = 0.0f;
			flow[i].line_wrap_extent  = 0.0f;
			flow[i].line_has_children = false;
			continue;
		}
		bool  can_wrap            = EgUiFlow_GetAxis(flow[i].wrap, &wrap_axis, &wrap_sign) && wrap_axis != primary_axis;
		float half_width          = bounds[i].w * 0.5f;
		float half_height         = bounds[i].h * 0.5f;
		float primary_limit       = primary_axis == 0 ? half_width : half_height;
		float wrap_limit          = wrap_axis == 0 ? half_width : half_height;
		flow[i].cursor_primary    = primary_sign > 0.0f ? -primary_limit : primary_limit;
		flow[i].cursor_wrap       = can_wrap ? (wrap_sign > 0.0f ? -wrap_limit : wrap_limit) : 0.0f;
		flow[i].line_wrap_extent  = 0.0f;
		flow[i].line_has_children = false;
	}
}

static void EgUiFlow_Update(ecs_iter_t *it)
{
	ecs_world_t       *world           = it->world;
	EgUiFlow          *flow            = ecs_field_shared(it, EgUiFlow, 0);
	EgShapesRectangle *parent_rect     = ecs_field_shared(it, EgShapesRectangle, 1);
	Position2         *child_positions = ecs_field_self(it, Position2, 2);
	EgShapesRectangle *child_rects     = ecs_field_self(it, EgShapesRectangle, 3);

	int   primary_axis;
	float primary_sign;
	int   wrap_axis = 0;
	float wrap_sign = 0.0f;
	if (!EgUiFlow_GetAxis(flow->direction, &primary_axis, &primary_sign)) {
		return;
	}
	bool  can_wrap      = EgUiFlow_GetAxis(flow->wrap, &wrap_axis, &wrap_sign) && wrap_axis != primary_axis;
	float primary_limit = (primary_axis == 0 ? parent_rect->w : parent_rect->h) * 0.5f;

	for (int i = 0; i < it->count; ++i) {
		float child_size[2]   = {child_rects[i].w, child_rects[i].h};
		float primary_size    = child_size[primary_axis];
		float next_primary    = flow->cursor_primary + primary_sign * primary_size;
		bool  exceeds_primary = primary_sign > 0.0f ? next_primary > primary_limit : next_primary < -primary_limit;
		if (can_wrap && flow->line_has_children && exceeds_primary) {
			flow->cursor_wrap += wrap_sign * flow->line_wrap_extent;
			flow->cursor_primary    = primary_sign > 0.0f ? -primary_limit : primary_limit;
			flow->line_wrap_extent  = 0.0f;
			flow->line_has_children = false;
		}

		child_positions[i].x = 0.0f;
		child_positions[i].y = 0.0f;
		if (primary_axis == 0) {
			child_positions[i].x = flow->cursor_primary + primary_sign * child_size[0] * 0.5f;
		} else {
			child_positions[i].y = flow->cursor_primary + primary_sign * child_size[1] * 0.5f;
		}
		if (can_wrap) {
			if (wrap_axis == 0) {
				child_positions[i].x = flow->cursor_wrap + wrap_sign * child_size[0] * 0.5f;
			} else {
				child_positions[i].y = flow->cursor_wrap + wrap_sign * child_size[1] * 0.5f;
			}
			if (child_size[wrap_axis] > flow->line_wrap_extent) {
				flow->line_wrap_extent = child_size[wrap_axis];
			}
		}
		flow->cursor_primary += primary_sign * primary_size;
		flow->line_has_children = true;
	}
}

void EgUiImport(ecs_world_t *world)
{
	ECS_IMPORT(world, EgPhysics);
	ECS_IMPORT(world, EgShapes);
	ECS_IMPORT(world, EgSpatials);
	ECS_IMPORT(world, EgButtons);

	ECS_MODULE(world, EgUi);
	ecs_set_name_prefix(world, "EgUi");

	ECS_COMPONENT_DEFINE(world, EgUiFlow);
	ECS_COMPONENT_DEFINE(world, EgUiDirection);
	ECS_COMPONENT_DEFINE(world, EgUiTable);
	ECS_COMPONENT_DEFINE(world, EgUiCell);
	ECS_COMPONENT_DEFINE(world, EgUiButton);

	ecs_set_hooks(world, EgUiTable,
	{
	.ctor = EgUiTable_ctor,
	.dtor = EgUiTable_dtor,
	.move = EgUiTable_move,
	.copy = EgUiTable_copy,
	});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgUiCell),
	.members = {
	{.name = "row", .type = ecs_id(ecs_i32_t)},
	{.name = "col", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgUiButton),
	.members = {
	{.name = "hovered", .type = ecs_id(ecs_bool_t)},
	{.name = "held", .type = ecs_id(ecs_bool_t)},
	}});

	// Maps are not reflected; explicit offsets let scripts set the gaps.
	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgUiTable),
	.members = {
	{.name = "row_gap", .type = ecs_id(ecs_f32_t), .offset = offsetof(EgUiTable, row_gap)},
	{.name = "col_gap", .type = ecs_id(ecs_f32_t), .offset = offsetof(EgUiTable, col_gap)},
	}});

	ecs_enum_init(world,
	&(ecs_enum_desc_t){
	.entity    = ecs_id(EgUiDirection),
	.constants = {
	{.name = "None", .value = EgUiDirectionNone},
	{.name = "Right", .value = EgUiDirectionRight},
	{.name = "Left", .value = EgUiDirectionLeft},
	{.name = "Up", .value = EgUiDirectionUp},
	{.name = "Down", .value = EgUiDirectionDown},
	}});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgUiFlow),
	.members = {
	{.name = "direction", .type = ecs_id(EgUiDirection)},
	{.name = "wrap", .type = ecs_id(EgUiDirection)},
	{.name = "cursor_primary", .type = ecs_id(ecs_f32_t)},
	{.name = "cursor_wrap", .type = ecs_id(ecs_f32_t)},
	{.name = "line_wrap_extent", .type = ecs_id(ecs_f32_t)},
	{.name = "line_has_children", .type = ecs_id(ecs_bool_t)},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiFlow_Reset"}),
	.phase       = EcsOnUpdate,
	.callback    = EgUiFlow_Reset,
	.query.terms = {
	{.id = ecs_id(EgUiFlow), .inout = EcsInOut},
	{.id = ecs_id(EgShapesRectangle), .inout = EcsIn},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiFlow_Update"}),
	.phase       = EcsOnUpdate,
	.callback    = EgUiFlow_Update,
	.query.terms = {
	{.id = ecs_id(EgUiFlow), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsInOut},
	{.id = ecs_id(EgShapesRectangle), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsIn},
	{.id = ecs_id(Position2), .inout = EcsOut},
	{.id = ecs_id(EgShapesRectangle), .inout = EcsIn},
	}});

	// Systems run in declaration order: Reset, Measure, Finalize, Layout.
	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiTable_Reset"}),
	.phase       = EcsOnUpdate,
	.callback    = EgUiTable_Reset,
	.query.terms = {
	{.id = ecs_id(EgUiTable), .inout = EcsInOut},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiTable_Measure"}),
	.phase       = EcsOnUpdate,
	.callback    = EgUiTable_Measure,
	.query.terms = {
	{.first.id = EcsChildOf, .second.name = "$table"},
	{.id = ecs_id(EgUiTable), .src.name = "$table", .inout = EcsInOut},
	{.id = ecs_id(EgUiCell), .inout = EcsIn},
	{.id = ecs_id(EgShapesRectangle), .inout = EcsIn},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiTable_Finalize"}),
	.phase       = EcsOnUpdate,
	.callback    = EgUiTable_Finalize,
	.query.terms = {
	{.id = ecs_id(EgUiTable), .inout = EcsInOut},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiTable_Layout"}),
	.phase       = EcsOnUpdate,
	.callback    = EgUiTable_Layout,
	.query.terms = {
	{.first.id = EcsChildOf, .second.name = "$table"},
	{.id = ecs_id(EgUiTable), .src.name = "$table", .inout = EcsIn},
	{.id = ecs_id(EgUiCell), .inout = EcsIn},
	{.id = ecs_id(EgShapesRectangle), .inout = EcsIn},
	{.id = ecs_id(Position2), .inout = EcsOut},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiMouseHitTesting_Update"}),
	.phase       = EcsPreStore,
	.callback    = EgUiMouseHitTesting_Update,
	.query.terms = {
	{.id = ecs_id(Position2), .trav = ecs_id(EgPhysicsOverlapChecking), .src.id = EcsUp, .inout = EcsIn},
	{.id = ecs_id(EgShapesRectangle), .inout = EcsIn},
	{.id = ecs_id(WorldTransform3), .inout = EcsIn},
	{.id = ecs_id(EgUiButton), .inout = EcsInOut},
	{.id = ecs_id(EgButtonsState), .src.id = ecs_id(EgButtonsState), .inout = EcsIn},
	}});
}
