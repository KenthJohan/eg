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



#include "draw_points.h"
#include "draw_shapes.h"
#include "Graphics.h"

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
	ECS_IMPORT(world, Graphics);
	ECS_IMPORT(world, Cameras);

	ecs_singleton_set(app->world, Window, {.w = 0, .h = 0});

	ecs_entity_t cam = ecs_new_entity(world, "Camera");
	ecs_set(world, cam, Camera, {.fov = 45.0f});
	ecs_set(world, cam, Position3, {0});
	ecs_set(world, cam, Orientation, {QF32_IDENTITY});
	ecs_set(world, cam, Velocity3, {0});
	ecs_set(world, cam, Rotate3, {0});

	ecs_entity_t e0 = ecs_new_entity(world, "ShapeBuffer");
	ecs_set(world, e0, ShapeBuffer, {0});

	ecs_entity_t e1 = ecs_new_entity(world, "Torus1");
	ecs_set(world, e1, Torus, {.radius = 10, .ring_radius = 1, .rings = 52, .sides = 25, .random_colors = 1});
	ecs_set(world, e1, Position3, {.x = 0.0f, .y = 0.0f, .z = 0.0f});
	ecs_add(world, e1, Transformation);
	ecs_add_pair(world, e1, EcsChildOf, e0);
	ecs_add_pair(world, e1, Use, cam);

	ecs_entity_t e2 = ecs_new_entity(world, "Torus2");
	ecs_set(world, e2, Torus, {.radius = 60, .ring_radius = 10, .rings = 52, .sides = 25, .random_colors = 1});
	ecs_add(world, e2, Transformation);
	ecs_set(world, e2, Position3, {.x = 00.0f, .y = 0.0f, .z = 100.0f});
	ecs_add_pair(world, e2, EcsChildOf, e0);
	ecs_add_pair(world, e2, Use, cam);


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
	        .clear_value = {0.0f, 0.2f, 0.4f, 1.0f}}};

	sdtx_canvas(w * 0.5f, h * 0.5f);
	sdtx_origin(1.0f, 2.0f);
	sdtx_color3f(1.0f, 1.0f, 1.0f);
	sdtx_printf("FPS: %f", 1.0f / dt);

	sg_begin_default_passf(&action1, w, h);
	{
		ecs_progress(app->world, 0.0f);
		sdtx_draw();
	}
	sg_end_pass();

	sg_commit();
}

static void event_cb(const sapp_event *evt, app_t *app)
{
	Window *window = ecs_singleton_get_mut(app->world, Window);
	uint8_t *keys = window->keys;

	switch (evt->type) {
	case SAPP_EVENTTYPE_MOUSE_DOWN:
		if (evt->mouse_button == SAPP_MOUSEBUTTON_LEFT) {
		}

		if (evt->mouse_button == SAPP_MOUSEBUTTON_RIGHT) {
		}
		break;
	case SAPP_EVENTTYPE_MOUSE_UP:
		if (evt->mouse_button == SAPP_MOUSEBUTTON_LEFT) {
		}

		if (evt->mouse_button == SAPP_MOUSEBUTTON_RIGHT) {
		}
		break;
	case SAPP_EVENTTYPE_MOUSE_SCROLL:
		break;
	case SAPP_EVENTTYPE_KEY_UP:
		assert(evt->key_code < 512);
		keys[evt->key_code] = 0;
		break;
	case SAPP_EVENTTYPE_KEY_DOWN:
		assert(evt->key_code < 512);
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
