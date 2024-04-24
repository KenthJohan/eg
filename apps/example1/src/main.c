#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_log.h>
#include <sokol_debugtext.h>
#include <sokol_glue.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <float.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>

#include <flecs.h>
#include <egcameras.h>
#include <egcomponents.h>
#include <egspatials.h>
#include <egsokol.h>

#include "MiscShapes.h"
#include "MiscLines.h"
#include "MiscPoints.h"
#include "MyController.h"

#include "argparse.h"




static void WindowLastFrame(ecs_iter_t *it)
{
	Window *window = ecs_field(it, Window, 1);
	window->mouse_left_edge = 0;
	window->mouse_right_edge = 0;
	memset(window->keys_edge, 0, sizeof(uint8_t)*512);
}


typedef struct {
	ecs_world_t *world;
} app_t;

static void init_cb(app_t *app)
{
	ecs_world_t *world = app->world;

	sg_setup(&(sg_desc){
        .environment = sglue_environment(),
        .logger.func = slog_func,
	});
	// setup sokol-debugtext
	sdtx_setup(&(sdtx_desc_t){
	.fonts[0] = sdtx_font_z1013(),
	.logger.func = slog_func,
	});

	ECS_IMPORT(world, EgComponents);
	ECS_IMPORT(world, EgSpatials);
	ECS_IMPORT(world, EgCameras);
	ECS_IMPORT(world, MiscShapes);
	ECS_IMPORT(world, MiscLines);
	ECS_IMPORT(world, MiscPoints);
	ECS_IMPORT(world, Sg);
	ECS_IMPORT(world, MyController);

	ecs_log_set_level(1);
	ecs_plecs_from_file(app->world, "config/keycode_sokol.flecs");
	ecs_plecs_from_file(app->world, "config/graphics_attributes.flecs");
	ecs_plecs_from_file(app->world, "config/graphics_pipes.flecs");
	ecs_plecs_from_file(app->world, "config/graphics_shaders.flecs");
	ecs_plecs_from_file(app->world, "config/graphics_ubs.flecs");
	ecs_plecs_from_file(app->world, "config/app.flecs");
	ecs_log_set_level(-1);



	// https://www.flecs.dev/explorer/?remote=true
	ecs_set(world, EcsWorld, EcsRest, {.port = 0});

	ECS_SYSTEM(world, WindowLastFrame, EcsPostUpdate, Window($));

	ecs_singleton_set(app->world, Window, {.w = 0, .h = 0});
}

static void frame_cb(app_t *app)
{
	float dt = sapp_frame_duration();
	float w = sapp_widthf();
	float h = sapp_heightf();
	Window *window = ecs_get_mut(app->world, ecs_id(Window), Window);
	window->w = w;
	window->h = h;
	window->dt = dt;

	sg_pass_action action1 = (sg_pass_action){
	.colors[0] = {
	.load_action = SG_LOADACTION_CLEAR,
	.clear_value = {0.1f, 0.1f, 0.1f, 1.0f}}};

	sg_begin_pass(&(sg_pass){ .action = action1, .swapchain = sglue_swapchain() });
	ecs_progress(app->world, 0.0f);
	sdtx_draw();
	sg_end_pass();
	sg_commit();
}

// https://github.com/floooh/sokol/blob/fa3d7cbe9ca85b8b87824ac366c724cb0e33a6af/util/sokol_nuklear.h#L2567

static void event_cb(const sapp_event *evt, app_t *app)
{
	Window *window = ecs_get_mut(app->world, ecs_id(Window), Window);
	uint8_t *keys = window->keys;
	uint8_t *keyse = window->keys_edge;

	switch (evt->type) {
	case SAPP_EVENTTYPE_MOUSE_DOWN:
		if (evt->mouse_button == SAPP_MOUSEBUTTON_LEFT) {
			if (window->mouse_left == 0) {
				window->mouse_left_edge = 1;
			}
			window->mouse_left = 1;
		}

		if (evt->mouse_button == SAPP_MOUSEBUTTON_RIGHT) {

		}
		break;
	case SAPP_EVENTTYPE_MOUSE_UP:
		if (evt->mouse_button == SAPP_MOUSEBUTTON_LEFT) {
			window->mouse_left = 0;
		}

		if (evt->mouse_button == SAPP_MOUSEBUTTON_RIGHT) {
		}
		break;
	case SAPP_EVENTTYPE_MOUSE_SCROLL:
		break;
	case SAPP_EVENTTYPE_MOUSE_MOVE:
		window->mouse_x = evt->mouse_x;
		window->mouse_y = evt->mouse_y;
		window->mouse_dx = evt->mouse_dx;
		window->mouse_dy = evt->mouse_dy;
		break;
	case SAPP_EVENTTYPE_KEY_UP:
		assert(evt->key_code < 512);
		keys[evt->key_code] = 0;
		break;
	case SAPP_EVENTTYPE_KEY_DOWN:
		assert(evt->key_code < 512);
		if (keys[evt->key_code] == 0) {
			keyse[evt->key_code] = 1;
		}
		keys[evt->key_code] = 1;
		break;
	case SAPP_EVENTTYPE_RESIZED: {
		break;
	}
	default:
		break;
	}

	if (keys[SAPP_KEYCODE_ESCAPE]) {
		sapp_quit();
	}
}

static void cleanup_cb(app_t *app)
{
	sdtx_shutdown();
	sg_shutdown();
	ecs_fini(app->world);
	free(app);
}


sapp_desc sokol_main(int argc, char /*const*/ *argv[])
{

	app_t *app = calloc(1, sizeof(app_t));
	app->world = ecs_init();
	// Disables warnings
	// Disables warnings about flecs HTTP request to timeout.
	ecs_log_set_level(-3);

	return (sapp_desc){
	.user_data = app,
	.init_userdata_cb = (void (*)(void *))init_cb,
	.frame_userdata_cb = (void (*)(void *))frame_cb,
	.cleanup_userdata_cb = (void (*)(void *))cleanup_cb,
	.event_userdata_cb = (void (*)(const sapp_event *, void *))event_cb,
	.width = 800,
	.height = 600,
	.sample_count = 4,
	.window_title = "example1",
	.icon.sokol_default = true,
	.logger.func = slog_func,
	};
}
