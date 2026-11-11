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
	int begins;
	int ends;
	bool valid;
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

static bool capture_begin(ecs_world_t *w, void *ctx)
{
	(void)w;
	TestPicker *picker = ctx;
	++picker->begins;
	return true;
}

static bool capture_valid(ecs_world_t *w, void *ctx)
{
	(void)w;
	return ((TestPicker *)ctx)->valid;
}

static void capture_end(ecs_world_t *w, void *ctx)
{
	(void)w;
	++((TestPicker *)ctx)->ends;
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

typedef struct {
	int32_t drags;
	float x;
} CaptureParentState;

typedef struct {
	int32_t releases;
} CaptureHandleState;

static void capture_to_local(ecs_world_t *w, ecs_entity_t entity,
	float sx, float sy, float *x, float *y, void *ctx)
{
	(void)sx; (void)sy; (void)ctx;
	const EgUiLocalMousePosition *local = ecs_get(w, entity, EgUiLocalMousePosition);
	if (local) {
		*x = local->x;
		*y = local->y;
	}
}

static ecs_entity_t create_capture_script(ecs_entity_t *parent)
{
	ecs_entity_t script = ecs_script(world, {.code =
		"using flecs.script\n"
		"template CaptureHandle {\n"
		"  eg.ui.script.events.Target\n"
		"  eg.ui.script.events.Capture\n"
		"  mut releases: i32 = 0\n"
		"  async {\n"
		"    while true {\n"
		"      await on.release(this)\n"
		"      releases = releases + 1\n"
		"    }\n"
		"  }\n"
		"}\n"
		"template CaptureParent {\n"
		"  mut drags: i32 = 0\n"
		"  mut x: f32 = 0\n"
		"  async {\n"
		"    while true {\n"
		"      const e = await on.drag(this)\n"
		"      drags = drags + 1\n"
		"      x = e.local_x\n"
		"    }\n"
		"  }\n"
		"  handle { CaptureHandle }\n"
		"}\n"
		"CaptureParent panel\n"});
	test_assert(script != 0);
	*parent = ecs_lookup(world, "panel");
	ecs_entity_t handle = ecs_lookup(world, "panel.handle");
	test_assert(*parent != 0 && handle != 0);
	ecs_set(world, *parent, EgUiLocalMousePosition, {.x = 5});
	ecs_script_set_event_hooks(world, &(ecs_script_event_hooks_t){.to_local = capture_to_local});
	ecs_script_tasks_progress(world);
	return handle;
}

void MouseHitTesting_test_capture_drag_release(void)
{
	ecs_entity_t parent;
	ecs_entity_t handle = create_capture_script(&parent);
	TestPicker picker = {.target = handle, .handled = true, .valid = true};
	ecs_singleton_set(world, EgUiScriptEventsPicker, {
		.pick = pick_test_target, .ctx = &picker, .capture_begin = capture_begin,
		.capture_valid = capture_valid, .capture_end = capture_end});
	ecs_singleton_set(world, EgButtonsState, {.mouse = {[1] = EG_BUTTONS_STATE_HELD}});
	ecs_progress(world, 0);
	test_int(picker.begins, 1);
	picker.target = 0;
	ecs_set(world, parent, EgUiLocalMousePosition, {.x = 55});
	ecs_progress(world, 0);
	const CaptureParentState *p = ecs_get_id(world, parent, ecs_lookup(world, "CaptureParent.mut"));
	test_assert(p != NULL);
	test_int(p->drags, 2);
	test_flt(p->x, 55);
	EcsScriptMouseEvent last = {0};
	test_assert(ecs_script_get_mouse_state(world, &last, NULL));
	test_assert(last.target == handle && last.buttons == 1);
	ecs_entity_t other = ecs_new(world);
	picker.target = other;
	ecs_progress(world, 0);
	test_assert(ecs_script_get_mouse_state(world, &last, NULL));
	test_assert(last.target == handle && last.buttons == 1);
	picker.target = 0;
	// Another held button must not steal capture or produce a synthetic press later.
	ecs_singleton_set(world, EgButtonsState, {.mouse = {[1] = EG_BUTTONS_STATE_HELD, [3] = EG_BUTTONS_STATE_HELD}});
	ecs_progress(world, 0);
	ecs_singleton_set(world, EgButtonsState, {.mouse = {[3] = EG_BUTTONS_STATE_HELD}});
	ecs_progress(world, 0);
	const CaptureHandleState *h = ecs_get_id(world, handle, ecs_lookup(world, "CaptureHandle.mut"));
	test_assert(h != NULL);
	test_int(h->releases, 1);
	test_int(picker.ends, 1);
	ecs_progress(world, 0);
	test_assert(ecs_script_get_mouse_state(world, &last, NULL));
	test_assert(last.target == 0 && last.buttons == 0);
	ecs_singleton_set(world, EgButtonsState, {{0}});
	ecs_progress(world, 0);
}

void MouseHitTesting_test_capture_cancellation(void)
{
	ecs_entity_t parent;
	ecs_entity_t handle = create_capture_script(&parent);
	TestPicker picker = {.target = handle, .handled = true, .valid = true};
	ecs_singleton_set(world, EgUiScriptEventsPicker, {
		.pick = pick_test_target, .ctx = &picker, .capture_begin = capture_begin,
		.capture_valid = capture_valid, .capture_end = capture_end});
	ecs_singleton_set(world, EgButtonsState, {.mouse = {[1] = EG_BUTTONS_STATE_HELD}});
	ecs_progress(world, 0);
	picker.valid = false;
	picker.target = 0;
	ecs_progress(world, 0);
	const CaptureHandleState *h = ecs_get_id(world, handle, ecs_lookup(world, "CaptureHandle.mut"));
	test_assert(h != NULL);
	test_int(h->releases, 1);
	test_int(picker.ends, 1);
	ecs_progress(world, 0);
	EcsScriptMouseEvent last = {0};
	test_assert(ecs_script_get_mouse_state(world, &last, NULL));
	test_assert(last.target == 0 && last.buttons == 0);
	ecs_singleton_set(world, EgButtonsState, {{0}});
	ecs_progress(world, 0);
	picker.valid = true;
	picker.target = handle;
	ecs_singleton_set(world, EgButtonsState, {.mouse = {[1] = EG_BUTTONS_STATE_HELD}});
	ecs_progress(world, 0);
	ecs_add_id(world, parent, EcsDisabled);
	picker.target = 0;
	ecs_progress(world, 0);
	test_int(picker.ends, 2);
	h = ecs_get_id(world, handle, ecs_lookup(world, "CaptureHandle.mut"));
	test_assert(h != NULL);
	test_int(h->releases, 2);
	ecs_remove_id(world, parent, EcsDisabled);
	ecs_singleton_set(world, EgButtonsState, {{0}});
	ecs_progress(world, 0);
	picker.target = handle;
	ecs_singleton_set(world, EgButtonsState, {.mouse = {[1] = EG_BUTTONS_STATE_HELD}});
	ecs_progress(world, 0);
	ecs_delete(world, handle);
	picker.target = 0;
	ecs_progress(world, 0);
	test_int(picker.ends, 3);
	ecs_singleton_set(world, EgButtonsState, {{0}});
	ecs_progress(world, 0);
	ecs_entity_t other = ecs_new(world);
	ecs_add_id(world, other, EgUiScriptEventsTarget);
	ecs_add_id(world, other, EgUiScriptEventsCapture);
	picker.target = other;
	ecs_singleton_set(world, EgButtonsState, {.mouse = {[1] = EG_BUTTONS_STATE_HELD}});
	ecs_progress(world, 0);
	ecs_entity_t system = ecs_lookup(world, "eg.ui.script.events.Emit");
	test_assert(system != 0);
	ecs_delete(world, system);
	test_int(picker.ends, 4);
}
