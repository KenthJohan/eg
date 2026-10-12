#include <flecs.h>
#include <stdio.h>

// Shows async/await and mouse events in flecs script.
//
// The script declares a Checkbox template whose `checked` state is a `mut`
// variable toggled by an `async` block that awaits `on.click(this)`.
// No C component is needed for the widget logic. The host only forwards mouse
// events with ecs_script_mouse_event() and the script reacts to them.
// The state is exposed by flecs as the component `Checkbox.mut`.

static const char *script =
"using flecs.script\n"
"template Checkbox {\n"
"  mut checked = false\n"
"  mut hover = false\n"
"  async {\n"
"    while true {\n"
"      await on.click(this)\n"
"      checked = !checked\n"
"    }\n"
"  }\n"
"  async {\n"
"    while true {\n"
"      await on.enter(this)\n"
"      hover = true\n"
"      await on.leave(this)\n"
"      hover = false\n"
"    }\n"
"  }\n"
"}\n"
"Checkbox my_checkbox\n"
"Checkbox other\n";

static void print_state(ecs_world_t *world, const char *step, ecs_entity_t e)
{
	ecs_entity_t mut = ecs_lookup(world, "Checkbox.mut");
	const void  *ptr = ecs_get_id(world, e, mut);
	char        *str = ptr ? ecs_ptr_to_expr(world, mut, ptr) : NULL;
	printf("%-16s %-12s %s\n", step, ecs_get_name(world, e), str ? str : "(none)");
	ecs_os_free(str);
}

static void mouse(ecs_world_t *world, ecs_entity_t target, uint32_t buttons, int32_t button)
{
	EcsScriptMouseEvent evt = {.target = target, .buttons = buttons, .button = button};
	ecs_script_mouse_event(world, &evt);
}

int main(int argc, char *argv[])
{
	ecs_world_t *world = ecs_init_w_args(argc, argv);
	ECS_IMPORT(world, FlecsScriptEvent);

	if (ecs_script_run(world, "async_example", script, NULL)) {
		return 1;
	}
	ecs_progress(world, 0);

	ecs_entity_t cb    = ecs_lookup(world, "my_checkbox");
	ecs_entity_t other = ecs_lookup(world, "other");
	print_state(world, "initial", cb);

	mouse(world, cb, 0, 0); // pointer enters the checkbox
	print_state(world, "pointer enter", cb);

	for (int i = 1; i <= 3; ++i) {
		mouse(world, cb, 1, 0); // press left button
		mouse(world, cb, 0, 0); // release on same entity -> click
		char step[32];
		snprintf(step, sizeof(step), "click %d", i);
		print_state(world, step, cb);
	}

	mouse(world, other, 0, 0); // pointer moves to another entity -> leave
	print_state(world, "pointer leave", cb);
	print_state(world, "other", other);

	return ecs_fini(world);
}
