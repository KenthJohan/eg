#include "EgUi.h"

#include <EgShapes.h>
#include <EgPhysics.h>
#include <EgButtons.h>
#include <EgShapedraw.h>
#include <ecsx.h>
#include <math.h>
#include <string.h>

ECS_COMPONENT_DECLARE(EgUiFlow);
ECS_COMPONENT_DECLARE(EgUiDirection);
ECS_COMPONENT_DECLARE(EgUiTable);
ECS_COMPONENT_DECLARE(EgUiCell);
ECS_COMPONENT_DECLARE(EgUiButton);
ECS_COMPONENT_DECLARE(EgUiResizable);
ECS_COMPONENT_DECLARE(EgUiTableGrid);
ECS_COMPONENT_DECLARE(EgUiAnchorKind);
ECS_COMPONENT_DECLARE(EgUiAnchor);

// Unit direction of an anchor kind, +y is up.
static void EgUiAnchorKind_Dir(EgUiAnchorKind k, float *x, float *y)
{
	static const float dir[][2] = {
	{0, 0}, {-1, 1}, {0, 1}, {1, 1}, {-1, 0}, {1, 0}, {-1, -1}, {0, -1}, {1, -1}};
	if ((int)k < 0 || (int)k > EgUiAnchorKindBottomRight) {
		k = EgUiAnchorKindMiddle;
	}
	*x = dir[k][0];
	*y = dir[k][1];
}

static void EgUiAnchor_Update(ecs_iter_t *it)
{
	EgUiAnchor              *a      = ecs_field_self(it, EgUiAnchor, 0);
	EgShapesRectangle const *r      = ecs_field_self(it, EgShapesRectangle, 1);
	Scale2 const            *scl    = ecs_field_self(it, Scale2, 2);
	Position2               *pos    = ecs_field_self(it, Position2, 3);
	EgShapesRectangle const *parent = ecs_field_is_set(it, 4) ? ecs_field_shared(it, EgShapesRectangle, 4) : NULL;

	float pw = parent ? parent->w * 0.5f : 0.0f;
	float ph = parent ? parent->h * 0.5f : 0.0f;
	for (int32_t i = 0; i < it->count; ++i) {
		float ax, ay, vx, vy;
		EgUiAnchorKind_Dir(a[i].parent, &ax, &ay);
		EgUiAnchorKind_Dir(a[i].pivot, &vx, &vy);
		pos[i].x = ax * pw + a[i].x - vx * r[i].w * 0.5f * scl[i].x;
		pos[i].y = ay * ph + a[i].y - vy * r[i].h * 0.5f * scl[i].y;
	}
}

static void EgUiMouseHitTesting_Update(ecs_iter_t *it)
{
	Position2               *p0 = ecs_field_shared(it, Position2, 0);
	EgShapesRectangle const *r  = ecs_field_self(it, EgShapesRectangle, 1);
	WorldTransform3 const   *x  = ecs_field_self(it, WorldTransform3, 2);
	EgUiButton              *b  = ecs_field_self(it, EgUiButton, 3);
	EgButtonsState          *s  = ecs_field_shared(it, EgButtonsState, 4);
	for (int32_t i = 0; i < it->count; ++i, ++r, ++x, ++b) {
		char const *name = ecs_get_name(it->world, it->entities[i]);
		float       dx   = p0->x - x->matrix.c2[0];
		float       dy   = p0->y - x->matrix.c2[1];
		float       det  = x->matrix.c0[0] * x->matrix.c1[1] - x->matrix.c1[0] * x->matrix.c0[1];
		if (fabsf(det) < 1e-8f) {
			continue;
		}
		float local_x    = (dx * x->matrix.c1[1] - dy * x->matrix.c1[0]) / det;
		float local_y    = (dy * x->matrix.c0[0] - dx * x->matrix.c0[1]) / det;
		bool  mouse_held = !!(EgButtonsState_get(s, b->key) & EG_BUTTONS_STATE_HELD);
		b->hovered       = (fabsf(local_x) <= fabsf(r->w) * 0.5f) && (fabsf(local_y) <= fabsf(r->h) * 0.5f);
		b->held          = (b->held && mouse_held) || (b->hovered && mouse_held);
		ecs_modified_id(it->world, it->entities[i], ecs_id(EgUiButton));
	}
}

static float EgUiTable_MapGet(const ecs_map_t *map, int32_t key);
static void  EgUiTable_MapSet(ecs_map_t *map, int32_t key, float value);

static void EgUiResizable_Update(ecs_iter_t *it)
{
	Position2             *mouse = ecs_field_shared(it, Position2, 0);
	EgShapesRectangle     *r     = ecs_field_self(it, EgShapesRectangle, 1);
	WorldTransform3 const *x     = ecs_field_self(it, WorldTransform3, 2);
	EgUiResizable         *z     = ecs_field_self(it, EgUiResizable, 3);
	EgButtonsState        *s     = ecs_field_shared(it, EgButtonsState, 4);
	Position2             *pos   = ecs_field_self(it, Position2, 5);
	Rotation2 const       *rot   = ecs_field_self(it, Rotation2, 6);
	Scale2 const          *scl   = ecs_field_self(it, Scale2, 7);
	EgShapesRectangle const *parent = ecs_field_is_set(it, 8) ? ecs_field_shared(it, EgShapesRectangle, 8) : NULL;
	EgUiAnchor              *anc    = ecs_field_is_set(it, 9) ? ecs_field_self(it, EgUiAnchor, 9) : NULL;

	for (int32_t i = 0; i < it->count; ++i) {
		float dx  = mouse->x - x[i].matrix.c2[0];
		float dy  = mouse->y - x[i].matrix.c2[1];
		float det = x[i].matrix.c0[0] * x[i].matrix.c1[1] - x[i].matrix.c1[0] * x[i].matrix.c0[1];
		if (fabsf(det) < 1e-8f) {
			continue;
		}
		// Rectangle-local mouse position (rotation and scale removed)
		float lx = (dx * x[i].matrix.c1[1] - dy * x[i].matrix.c1[0]) / det;
		float ly = (dy * x[i].matrix.c0[0] - dx * x[i].matrix.c0[1]) / det;

		float hw   = r[i].w * 0.5f;
		float hh   = r[i].h * 0.5f;
		bool  held = !!(EgButtonsState_get(s, z[i].key) & EG_BUTTONS_STATE_HELD);

		uint8_t prev_edge = z[i].edge;
		bool    prev_drag = z[i].dragging;

		if (!z[i].dragging) {
			bool    in_x = fabsf(lx) <= hw + z[i].grab;
			bool    in_y = fabsf(ly) <= hh + z[i].grab;
			uint8_t e    = 0;
			if (in_y && fabsf(lx + hw) <= z[i].grab)
				e |= EG_UI_EDGE_LEFT;
			if (in_y && fabsf(lx - hw) <= z[i].grab)
				e |= EG_UI_EDGE_RIGHT;
			if (in_x && fabsf(ly + hh) <= z[i].grab)
				e |= EG_UI_EDGE_BOTTOM;
			if (in_x && fabsf(ly - hh) <= z[i].grab)
				e |= EG_UI_EDGE_TOP;
			// The side pinned by the anchor pivot is not draggable, otherwise the inset would change
			if (anc) {
				float vx, vy;
				EgUiAnchorKind_Dir(anc[i].pivot, &vx, &vy);
				if (vx > 0.0f)
					e &= (uint8_t)~EG_UI_EDGE_RIGHT;
				if (vx < 0.0f)
					e &= (uint8_t)~EG_UI_EDGE_LEFT;
				if (vy > 0.0f)
					e &= (uint8_t)~EG_UI_EDGE_TOP;
				if (vy < 0.0f)
					e &= (uint8_t)~EG_UI_EDGE_BOTTOM;
			}
			z[i].edge = e;
			// Only a fresh press starts a drag
			if (held && !z[i].was_held && e) {
				z[i].dragging = true;
				z[i].offset_x = (e & EG_UI_EDGE_RIGHT) ? lx - hw : (e & EG_UI_EDGE_LEFT) ? lx + hw
				                                                                         : 0.0f;
				z[i].offset_y = (e & EG_UI_EDGE_TOP) ? ly - hh : (e & EG_UI_EDGE_BOTTOM) ? ly + hh
				                                                                         : 0.0f;
			}
		}
		z[i].was_held = held;

		if (z[i].dragging && !held) {
			z[i].dragging = false;
			z[i].edge     = 0;
		}

		if (z[i].dragging) {
			uint8_t e  = z[i].edge;
			float   mx = lx - z[i].offset_x;
			float   my = ly - z[i].offset_y;
			float   dw = 0.0f;
			float   dh = 0.0f;
			if (e & EG_UI_EDGE_RIGHT)
				dw = mx - hw;
			if (e & EG_UI_EDGE_LEFT)
				dw = -(mx + hw);
			if (e & EG_UI_EDGE_TOP)
				dh = my - hh;
			if (e & EG_UI_EDGE_BOTTOM)
				dh = -(my + hh);

			// Middle pivot keeps the center, so both sides move and the edge follows the mouse
			if (anc) {
				float vx, vy;
				EgUiAnchorKind_Dir(anc[i].pivot, &vx, &vy);
				if (vx == 0.0f)
					dw *= 2.0f;
				if (vy == 0.0f)
					dh *= 2.0f;
			}

			dw = fmaxf(dw, z[i].min_w - r[i].w);
			dh = fmaxf(dh, z[i].min_h - r[i].h);

			ecs_entity_t parent_ent = ecs_get_parent(it->world, it->entities[i]);
			bool         centered   = parent_ent && ecs_has_id(it->world, parent_ent, ecs_id(EgUiFlow));

			// Moving edge may not pass the parent boundary; ignores rotation. Never forces a shrink.
			if (parent && centered) {
				float max_w = 2.0f * fmaxf(0.0f, parent->w * 0.5f - fabsf(pos[i].x)) / scl[i].x;
				float max_h = 2.0f * fmaxf(0.0f, parent->h * 0.5f - fabsf(pos[i].y)) / scl[i].y;
				if (e & (EG_UI_EDGE_LEFT | EG_UI_EDGE_RIGHT)) {
					dw = fminf(dw, fmaxf(0.0f, max_w - r[i].w));
				}
				if (e & (EG_UI_EDGE_TOP | EG_UI_EDGE_BOTTOM)) {
					dh = fminf(dh, fmaxf(0.0f, max_h - r[i].h));
				}
			} else if (parent) {
				float pw = parent->w * 0.5f;
				float ph = parent->h * 0.5f;
				if ((e & EG_UI_EDGE_RIGHT) && scl[i].x > 0.0f) {
					float max_w = (pw - (pos[i].x - hw * scl[i].x)) / scl[i].x;
					dw          = fminf(dw, fmaxf(0.0f, max_w - r[i].w));
				}
				if ((e & EG_UI_EDGE_LEFT) && scl[i].x > 0.0f) {
					float max_w = ((pos[i].x + hw * scl[i].x) + pw) / scl[i].x;
					dw          = fminf(dw, fmaxf(0.0f, max_w - r[i].w));
				}
				if ((e & EG_UI_EDGE_TOP) && scl[i].y > 0.0f) {
					float max_h = (ph - (pos[i].y - hh * scl[i].y)) / scl[i].y;
					dh          = fminf(dh, fmaxf(0.0f, max_h - r[i].h));
				}
				if ((e & EG_UI_EDGE_BOTTOM) && scl[i].y > 0.0f) {
					float max_h = ((pos[i].y + hh * scl[i].y) + ph) / scl[i].y;
					dh          = fminf(dh, fmaxf(0.0f, max_h - r[i].h));
				}
			}

			// Opposite side stays fixed: size changes by d, center by d/2
			float sx = (e & EG_UI_EDGE_RIGHT) ? 0.5f : (e & EG_UI_EDGE_LEFT) ? -0.5f
			                                                                 : 0.0f;
			float sy = (e & EG_UI_EDGE_TOP) ? 0.5f : (e & EG_UI_EDGE_BOTTOM) ? -0.5f
			                                                                 : 0.0f;
			float px = sx * dw * scl[i].x;
			float py = sy * dh * scl[i].y;
			float c  = cosf(rot[i].radians);
			float sn = sinf(rot[i].radians);

			r[i].w += dw;
			r[i].h += dh;
			if (anc) {
				// The pivot side stays put and the anchor rebuilds Position2, so the offset is unchanged
			} else if (centered) {
				// Position2 is rewritten by the layout
			} else {
				pos[i].x += px * c - py * sn;
				pos[i].y += px * sn + py * c;
			}
		}

		if (z[i].edge != prev_edge || z[i].dragging != prev_drag) {
			ecs_modified_id(it->world, it->entities[i], ecs_id(EgUiResizable));
		}
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

static void EgUiTable_MapSet(ecs_map_t *map, int32_t key, float value)
{
	ecs_map_val_t *val = ecs_map_ensure(map, (ecs_map_key_t)(uint32_t)key);
	*val               = 0;
	memcpy(val, &value, sizeof(value));
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
	EgUiTable *table = ecs_field_shared(it, EgUiTable, 1);
	EgUiCell  *cells = ecs_field_self(it, EgUiCell, 2);
	for (int i = 0; i < it->count; ++i) {
		if (cells[i].row < 0 || cells[i].col < 0) {
			continue;
		}
		if (cells[i].row + 1 > table->row_count) {
			table->row_count = cells[i].row + 1;
		}
		if (cells[i].col + 1 > table->col_count) {
			table->col_count = cells[i].col + 1;
		}
	}
}

static void EgUiTable_ResolveTracks(ecs_map_t *resolved, int32_t count, float gap, float extent)
{
	float share = count > 0 ? fmaxf(0.0f, extent - gap * (float)(count - 1)) / (float)count : 0.0f;
	for (int32_t k = 0; k < count; ++k) {
		EgUiTable_MapSet(resolved, k, share);
	}
}

static void EgUiTable_Finalize(ecs_iter_t *it)
{
	EgUiTable               *table = ecs_field_self(it, EgUiTable, 0);
	EgShapesRectangle const *rect  = ecs_field_self(it, EgShapesRectangle, 1);
	for (int i = 0; i < it->count; ++i) {
		EgUiTable_ResolveTracks(&table[i].cols_width, table[i].col_count, table[i].col_gap, rect[i].w);
		EgUiTable_ResolveTracks(&table[i].rows_height, table[i].row_count, table[i].row_gap, rect[i].h);
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
		// The cell fills its slot; table is centered on the parent origin, rows grow downward (+y is up).
		rects[i].w     = col_w;
		rects[i].h     = row_h;
		positions[i].x = -table->total_space.w * 0.5f + left + col_w * 0.5f;
		positions[i].y = table->total_space.h * 0.5f - top - row_h * 0.5f;
	}
}

static void EgUiTableGrid_Draw(ecs_iter_t *it)
{
	EgShapedrawList       *l  = ecs_field_shared(it, EgShapedrawList, 0);
	EgUiTable const       *t  = ecs_field_self(it, EgUiTable, 1);
	EgUiTableGrid const   *g  = ecs_field_self(it, EgUiTableGrid, 2);
	WorldTransform3 const *w  = ecs_field_self(it, WorldTransform3, 3);
	EgShapedrawZ const    *zs = ecs_field_self(it, EgShapedrawZ, 4);

	for (int i = 0; i < it->count; ++i) {
		int32_t z  = zs ? zs[i].z : 0;
		float   hw = t[i].total_space.w * 0.5f;
		float   hh = t[i].total_space.h * 0.5f;

		// Separators sit in the middle of each gap between neighbouring slots
		for (int32_t c = 0; c + 1 < t[i].col_count; ++c) {
			float x = -hw + EgUiTable_Offset(&t[i].cols_width, c + 1, t[i].col_gap) - t[i].col_gap * 0.5f;
			EgShapedrawList_AddLine(l, z, &w[i].matrix, x, -hh, x, hh, g[i].thickness, g[i].color);
		}
		for (int32_t r = 0; r + 1 < t[i].row_count; ++r) {
			float y = hh - EgUiTable_Offset(&t[i].rows_height, r + 1, t[i].row_gap) + t[i].row_gap * 0.5f;
			EgShapedrawList_AddLine(l, z, &w[i].matrix, -hw, y, hw, y, g[i].thickness, g[i].color);
		}
		if (g[i].outer) {
			EgShapedrawList_AddRectangleOutline(l, z, &w[i].matrix, t[i].total_space.w, t[i].total_space.h, g[i].thickness, g[i].color);
		}
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
	ECS_IMPORT(world, EgShapedraw);

	ECS_MODULE(world, EgUi);
	ecs_set_name_prefix(world, "EgUi");

	ECS_COMPONENT_DEFINE(world, EgUiFlow);
	ECS_COMPONENT_DEFINE(world, EgUiDirection);
	ECS_COMPONENT_DEFINE(world, EgUiTable);
	ECS_COMPONENT_DEFINE(world, EgUiCell);
	ECS_COMPONENT_DEFINE(world, EgUiButton);
	ECS_COMPONENT_DEFINE(world, EgUiResizable);
	ECS_COMPONENT_DEFINE(world, EgUiTableGrid);

	// The table layout writes the cell Rectangle, so every cell needs one.
	ecs_add_pair(world, ecs_id(EgUiCell), EcsWith, ecs_id(EgShapesRectangle));
	ECS_COMPONENT_DEFINE(world, EgUiAnchorKind);
	ECS_COMPONENT_DEFINE(world, EgUiAnchor);

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
	{.name = "key", .type = ecs_id(ecs_u32_t)},
	{.name = "hovered", .type = ecs_id(ecs_bool_t)},
	{.name = "held", .type = ecs_id(ecs_bool_t)},
	}});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgUiResizable),
	.members = {
	{.name = "key", .type = ecs_id(ecs_u32_t)},
	{.name = "grab", .type = ecs_id(ecs_f32_t)},
	{.name = "min_w", .type = ecs_id(ecs_f32_t)},
	{.name = "min_h", .type = ecs_id(ecs_f32_t)},
	{.name = "edge", .type = ecs_id(ecs_u8_t)},
	{.name = "dragging", .type = ecs_id(ecs_bool_t)},
	{.name = "was_held", .type = ecs_id(ecs_bool_t)},
	{.name = "offset_x", .type = ecs_id(ecs_f32_t)},
	{.name = "offset_y", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_enum_init(world,
	&(ecs_enum_desc_t){
	.entity    = ecs_id(EgUiAnchorKind),
	.constants = {
	{.name = "Middle", .value = EgUiAnchorKindMiddle},
	{.name = "TopLeft", .value = EgUiAnchorKindTopLeft},
	{.name = "Top", .value = EgUiAnchorKindTop},
	{.name = "TopRight", .value = EgUiAnchorKindTopRight},
	{.name = "Left", .value = EgUiAnchorKindLeft},
	{.name = "Right", .value = EgUiAnchorKindRight},
	{.name = "BottomLeft", .value = EgUiAnchorKindBottomLeft},
	{.name = "Bottom", .value = EgUiAnchorKindBottom},
	{.name = "BottomRight", .value = EgUiAnchorKindBottomRight},
	}});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgUiAnchor),
	.members = {
	{.name = "parent", .type = ecs_id(EgUiAnchorKind)},
	{.name = "pivot", .type = ecs_id(EgUiAnchorKind)},
	{.name = "x", .type = ecs_id(ecs_f32_t)},
	{.name = "y", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgUiTableGrid),
	.members = {
	{.name = "thickness", .type = ecs_id(ecs_f32_t)},
	{.name = "color", .type = ecs_id(ecs_u32_t)},
	{.name = "outer", .type = ecs_id(ecs_bool_t)},
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
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiTable_Finalize"}),
	.phase       = EcsOnUpdate,
	.callback    = EgUiTable_Finalize,
	.query.terms = {
	{.id = ecs_id(EgUiTable), .inout = EcsInOut},
	{.id = ecs_id(EgShapesRectangle), .src.id = EcsSelf, .inout = EcsIn},
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
	{.id = ecs_id(EgShapesRectangle), .inout = EcsOut},
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

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiResizable_Update"}),
	.phase       = EcsPreStore,
	.callback    = EgUiResizable_Update,
	.query.terms = {
	{.id = ecs_id(Position2), .trav = ecs_id(EgPhysicsOverlapChecking), .src.id = EcsUp, .inout = EcsIn},
	{.id = ecs_id(EgShapesRectangle), .inout = EcsInOut},
	{.id = ecs_id(WorldTransform3), .inout = EcsIn},
	{.id = ecs_id(EgUiResizable), .inout = EcsInOut},
	{.id = ecs_id(EgButtonsState), .src.id = ecs_id(EgButtonsState), .inout = EcsIn},
	{.id = ecs_id(Position2), .src.id = EcsSelf, .inout = EcsInOut},
	{.id = ecs_id(Rotation2), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(Scale2), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(EgShapesRectangle), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsIn, .oper = EcsOptional},
	{.id = ecs_id(EgUiAnchor), .src.id = EcsSelf, .inout = EcsInOut, .oper = EcsOptional},
	{.id = ecs_id(EgUiCell), .src.id = EcsSelf, .oper = EcsNot},
	}});

	// Declared after EgUiResizable_Update so a drag is applied before Position2 is rebuilt.
	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiAnchor_Update"}),
	.phase       = EcsPreStore,
	.callback    = EgUiAnchor_Update,
	.query.terms = {
	{.id = ecs_id(EgUiAnchor), .inout = EcsIn},
	{.id = ecs_id(EgShapesRectangle), .inout = EcsIn},
	{.id = ecs_id(Scale2), .inout = EcsIn},
	{.id = ecs_id(Position2), .src.id = EcsSelf, .inout = EcsOut},
	{.id = ecs_id(EgShapesRectangle), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsIn, .oper = EcsOptional},
	{.id = ecs_id(EgUiCell), .oper = EcsNot},
	{.id = ecs_id(EgUiFlow), .trav = EcsChildOf, .src.id = EcsUp, .oper = EcsNot},
	}});

	// Collected with the other shapes, after layout has run.
	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity      = ecs_entity(world, {.name = "EgUiTableGrid_Draw"}),
	.phase       = EcsPostUpdate,
	.callback    = EgUiTableGrid_Draw,
	.query.terms = {
	{.id = ecs_id(EgShapedrawList), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsInOut},
	{.id = ecs_id(EgUiTable), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(EgUiTableGrid), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(WorldTransform3), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(EgShapedrawZ), .src.id = EcsSelf, .inout = EcsIn, .oper = EcsOptional},
	}});
}
