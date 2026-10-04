#include <bake_test.h>
#include <EgShapes.h>
#include <EgSpatials.h>
#include <EgUi.h>
#include <math.h>

#define EPSILON 1e-4f

static ecs_world_t *world;

static ecs_entity_t make_cell(ecs_entity_t parent, int32_t row, int32_t col, float w, float h)
{
	ecs_entity_t e = ecs_new(world);
	ecs_add_pair(world, e, EcsChildOf, parent);
	ecs_set(world, e, EgUiCell, {row, col});
	ecs_set(world, e, EgShapesRectangle, {.w = w, .h = h});
	ecs_set(world, e, Position2, {0.0f, 0.0f});
	return e;
}

static void assert_pos(ecs_entity_t e, float x, float y)
{
	const Position2 *p = ecs_get(world, e, Position2);
	test_assert(p != NULL);
	test_assert(fabsf(p->x - x) < EPSILON);
	test_assert(fabsf(p->y - y) < EPSILON);
}

void TableLayout_setup(void)
{
	world = ecs_init();
	ECS_IMPORT(world, EgShapes);
	ECS_IMPORT(world, EgSpatials);
	ECS_IMPORT(world, EgUi);
}

void TableLayout_teardown(void)
{
	ecs_fini(world);
	world = NULL;
}

void TableLayout_test_cells_size_to_largest_child_and_position(void)
{
	ecs_entity_t table = ecs_new(world);
	ecs_set(world, table, EgUiTable, {.row_gap = 10.0f, .col_gap = 4.0f});

	ecs_entity_t a = make_cell(table, 0, 0, 40, 20);
	ecs_entity_t b = make_cell(table, 0, 1, 20, 30);
	ecs_entity_t c = make_cell(table, 1, 0, 10, 10);
	ecs_entity_t d = make_cell(table, 1, 1, 30, 10);

	ecs_progress(world, 0);

	const EgUiTable *t = ecs_get(world, table, EgUiTable);
	test_assert(t != NULL);
	test_int(t->row_count, 2);
	test_int(t->col_count, 2);
	test_assert(fabsf(t->total_space.w - 74.0f) < EPSILON);
	test_assert(fabsf(t->total_space.h - 50.0f) < EPSILON);

	assert_pos(a, -17.0f, 10.0f);
	assert_pos(b, 22.0f, 10.0f);
	assert_pos(c, -17.0f, -20.0f);
	assert_pos(d, 22.0f, -20.0f);
}

void TableLayout_test_empty_table_has_no_space(void)
{
	ecs_entity_t table = ecs_new(world);
	ecs_set(world, table, EgUiTable, {.row_gap = 5.0f, .col_gap = 5.0f});

	ecs_progress(world, 0);

	const EgUiTable *t = ecs_get(world, table, EgUiTable);
	test_int(t->row_count, 0);
	test_int(t->col_count, 0);
	test_assert(t->total_space.w == 0.0f);
	test_assert(t->total_space.h == 0.0f);
}

void TableLayout_test_sparse_cells_leave_empty_columns(void)
{
	ecs_entity_t table = ecs_new(world);
	ecs_set(world, table, EgUiTable, {0});
	ecs_entity_t a = make_cell(table, 0, 2, 10, 10);

	ecs_progress(world, 0);

	const EgUiTable *t = ecs_get(world, table, EgUiTable);
	test_int(t->col_count, 3);
	test_assert(fabsf(t->total_space.w - 10.0f) < EPSILON);
	assert_pos(a, 0.0f, 0.0f);
}

void TableLayout_test_relayout_after_child_resize(void)
{
	ecs_entity_t table = ecs_new(world);
	ecs_set(world, table, EgUiTable, {0});
	ecs_entity_t a = make_cell(table, 0, 0, 10, 10);

	ecs_progress(world, 0);
	ecs_set(world, a, EgShapesRectangle, {.w = 30, .h = 20});
	ecs_progress(world, 0);

	const EgUiTable *t = ecs_get(world, table, EgUiTable);
	test_assert(fabsf(t->total_space.w - 30.0f) < EPSILON);
	test_assert(fabsf(t->total_space.h - 20.0f) < EPSILON);
}

void TableLayout_test_nested_descendant_is_not_a_cell(void)
{
	ecs_entity_t table = ecs_new(world);
	ecs_set(world, table, EgUiTable, {0});
	ecs_entity_t intermediate = ecs_new(world);
	ecs_add_pair(world, intermediate, EcsChildOf, table);
	ecs_entity_t nested_cell = make_cell(intermediate, 0, 0, 100, 80);
	ecs_set(world, nested_cell, Position2, {7.0f, 9.0f});

	ecs_progress(world, 0);

	const EgUiTable *t = ecs_get(world, table, EgUiTable);
	test_assert(t != NULL);
	test_int(t->row_count, 0);
	test_int(t->col_count, 0);
	test_assert(t->total_space.w == 0.0f);
	test_assert(t->total_space.h == 0.0f);
	assert_pos(nested_cell, 7.0f, 9.0f);
}
