#include <bake_test.h>
#include <EgButtons.h>
#include <EgShapes.h>
#include <EgSpatials.h>
#include <EgUi.h>
#include <math.h>

static ecs_world_t *world;

typedef struct {
	ecs_entity_t target;
	bool handled;
	int calls;
} TestPicker;

static bool pick_test_target(ecs_world_t *w, ecs_entity_t *target, float *x, float *y, void *ctx)
{
	(void)w;
	TestPicker *picker = ctx;
	++picker->calls;
	*target = picker->target;
	*x = 13;
	*y = 17;
	return picker->handled;
}

void MouseHitTesting_setup(void)
{
	world = ecs_init();
	ECS_IMPORT(world, EgShapes);
	ECS_IMPORT(world, EgSpatials);
	ECS_IMPORT(world, EgUi);
	ecs_singleton_set(world, EgButtonsState, {{0}});
}

void MouseHitTesting_teardown(void)
{
	ecs_fini(world);
	world = NULL;
}

static ecs_entity_t make_rect(ecs_entity_t *mouse)
{
	*mouse = ecs_new(world);
	ecs_set(world, *mouse, Position2, {0.0f, 0.0f});
	ecs_entity_t root = ecs_new(world);
	ecs_add_pair(world, root, EgUiMouse, *mouse);
	ecs_entity_t rect = ecs_new_w_pair(world, EcsChildOf, root);
	ecs_set(world, rect, EgUiLocalMousePosition, {0});
	ecs_set(world, rect, EgShapesRectangle, {.w = 4, .h = 4});
	ecs_set(world, rect, WorldTransform3, {.matrix = M3_IDENTITY});
	ecs_add_id(world, rect, EgUiScriptEventsTarget);
	return rect;
}

void MouseHitTesting_test_external_pick_dispatch(void)
{
	ecs_entity_t mouse;
	ecs_entity_t rect = make_rect(&mouse);
	TestPicker picker = {.target = rect, .handled = true};
	ecs_singleton_set(world, EgUiScriptEventsPicker, {.pick = pick_test_target, .ctx = &picker});

	ecs_progress(world, 0);
	EcsScriptMouseEvent last = {0};
	test_assert(ecs_script_get_mouse_state(world, &last, NULL));
	test_assert(last.target == rect);
	test_assert(last.screen_x == 13 && last.screen_y == 17);
	uint64_t event_id = last.id;
	test_int(picker.calls, 1);

	picker.target = 0;
	ecs_progress(world, 0);
	test_assert(ecs_script_get_mouse_state(world, &last, NULL));
	test_assert(last.target == 0);
	test_assert(last.id == event_id + 1);

	picker.target = rect;
	ecs_singleton_set(world, EgButtonsState, {.mouse = {[1] = EG_BUTTONS_STATE_HELD}});
	ecs_progress(world, 0);
	ecs_entity_t pressed = 0;
	test_assert(ecs_script_get_mouse_state(world, &last, &pressed));
	test_assert(last.target == rect && pressed == rect && last.buttons == 1);
	picker.target = 0;
	ecs_progress(world, 0);
	test_assert(ecs_script_get_mouse_state(world, &last, &pressed));
	test_assert(last.target == 0 && pressed == rect);
	ecs_singleton_set(world, EgButtonsState, {{0}});
	ecs_progress(world, 0);
	test_assert(ecs_script_get_mouse_state(world, &last, &pressed));
	test_assert(last.target == 0 && last.buttons == 0);

	picker.target = rect;
	picker.handled = false;
	ecs_progress(world, 0);
	test_assert(ecs_script_get_mouse_state(world, &last, NULL));
	test_assert(last.target == 0);
	test_int(picker.calls, 6);

	picker.handled = true;
	ecs_progress(world, 0);
	test_assert(ecs_script_get_mouse_state(world, &last, NULL));
	test_assert(last.target == rect);
	ecs_singleton_remove(world, EgUiScriptEventsPicker);
	ecs_progress(world, 0);
	test_assert(ecs_script_get_mouse_state(world, &last, NULL));
	test_assert(last.target == 0);
}

void MouseHitTesting_test_no_picker_does_not_hit_rectangle(void)
{
	ecs_entity_t mouse;
	make_rect(&mouse);
	ecs_progress(world, 0);
	EcsScriptMouseEvent last = {0};
	test_assert(ecs_script_get_mouse_state(world, &last, NULL));
	test_assert(last.target == 0);
}

void MouseHitTesting_test_local_coordinates(void)
{
	ecs_entity_t mouse;
	ecs_entity_t rect = make_rect(&mouse);
	ecs_set(world, rect, WorldTransform3,
		{.matrix = {{0, 2, 0}, {-3, 0, 0}, {10, 20, 1}}});
	ecs_set(world, mouse, Position2, {7, 22});
	ecs_progress(world, 0);
	EgUiLocalMousePosition const *l = ecs_get(world, rect, EgUiLocalMousePosition);
	test_assert(fabsf(l->x - 1.0f) < 1e-4f);
	test_assert(fabsf(l->y - 1.0f) < 1e-4f);
	test_assert(fabsf(l->pixel_x - 0.5f) < 1e-4f);
	test_assert(fabsf(l->pixel_y - 1.0f / 3.0f) < 1e-4f);
}
