#include <sokol/sokol_app.h>
#include <sokol/sokol_gfx.h>
#include <sokol/sokol_log.h>
#include <sokol/sokol_debugtext.h>
#include <sokol/sokol_glue.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <float.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>

#include <flecs.h>
#include <eg/Cameras.h>
#include <eg/Components.h>
#include <egsokol/Sg.h>

#include "MiscShapes.h"
#include "MiscLines.h"
#include "MiscPoints.h"
#include "MyController.h"




static void WindowLastFrame(ecs_iter_t *it)
{
	Window *window = ecs_field(it, Window, 1);
	window->mouse_left_edge = 0;
}


typedef struct {
	ecs_world_t *world;
} app_t;

static void init_cb(app_t *app)
{
	ecs_world_t *world = app->world;

	sg_setup(&(sg_desc){
	.context = sapp_sgcontext(),
	.logger.func = slog_func,
	});
	// setup sokol-debugtext
	sdtx_setup(&(sdtx_desc_t){
	.fonts[0] = sdtx_font_z1013(),
	.logger.func = slog_func,
	});

	ECS_IMPORT(world, Components);
	ECS_IMPORT(world, MiscShapes);
	ECS_IMPORT(world, MiscLines);
	ECS_IMPORT(world, MiscPoints);
	ECS_IMPORT(world, Cameras);
	ECS_IMPORT(world, Sg);
	ECS_IMPORT(world, MyController);

	ecs_plecs_from_file(app->world, "config/graphics_attributes.flecs");
	ecs_plecs_from_file(app->world, "config/graphics_pipes.flecs");
	ecs_plecs_from_file(app->world, "config/graphics_shaders.flecs");
	ecs_plecs_from_file(app->world, "config/graphics_ubs.flecs");
	ecs_plecs_from_file(app->world, "config/app.flecs");

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
	Window *window = ecs_singleton_get_mut(app->world, Window);
	window->w = w;
	window->h = h;

	sg_pass_action action1 = (sg_pass_action){
	.colors[0] = {
	.load_action = SG_LOADACTION_CLEAR,
	.clear_value = {0.1f, 0.1f, 0.1f, 1.0f}}};

	sg_begin_default_passf(&action1, w, h);
	ecs_progress(app->world, 0.0f);
	sdtx_draw();
	sg_end_pass();
	sg_commit();
}

// https://github.com/floooh/sokol/blob/fa3d7cbe9ca85b8b87824ac366c724cb0e33a6af/util/sokol_nuklear.h#L2567

static void event_cb(const sapp_event *evt, app_t *app)
{
	Window *window = ecs_singleton_get_mut(app->world, Window);
	uint8_t *keys = window->keys;
	egsokol_flecs_event_cb(evt, window);
}

static void cleanup_cb(app_t *app)
{
	sdtx_shutdown();
	sg_shutdown();
	ecs_fini(app->world);
	free(app);
}

sapp_desc sokol_main(int argc, char *argv[])
{

	app_t *app = calloc(1, sizeof(app_t));

	app->world = ecs_init();

	return (sapp_desc){
	.user_data = app,
	.init_userdata_cb = (void (*)(void *))init_cb,
	.frame_userdata_cb = (void (*)(void *))frame_cb,
	.cleanup_userdata_cb = (void (*)(void *))cleanup_cb,
	.event_userdata_cb = (void (*)(const sapp_event *, void *))event_cb,
	.width = 800,
	.height = 600,
	.sample_count = 4,
	.window_title = "Primitive Types",
	.icon.sokol_default = true,
	.logger.func = slog_func,
	};
}
