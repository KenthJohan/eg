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


#include "Graphics.h"



void ControllerRotate(ecs_iter_t *it)
{
	// Camera *camera = ecs_field(it, Camera, 1);
	Rotate3 *rotate = ecs_field(it, Rotate3, 2);
	Window *window = ecs_field(it, Window, 3);
	uint8_t *keys = window->keys;
	float k = 0.8f * it->delta_time;
	for (int i = 0; i < it->count; i++) {
		rotate->dx = keys[SAPP_KEYCODE_UP] - keys[SAPP_KEYCODE_DOWN];
		rotate->dy = keys[SAPP_KEYCODE_RIGHT] - keys[SAPP_KEYCODE_LEFT];
		rotate->dz = keys[SAPP_KEYCODE_E] - keys[SAPP_KEYCODE_Q];
		v3f32_mul((float *)rotate, (float *)rotate, k);
	}
}

void ControllerMove(ecs_iter_t *it)
{
	// Camera *camera = ecs_field(it, Camera, 1);
	Velocity3 *vel = ecs_field(it, Velocity3, 2);
	Window *window = ecs_field(it, Window, 3);
	uint8_t *keys = window->keys;
	float moving_speed = 100.1f;
	float k = it->delta_time * moving_speed;
	for (int i = 0; i < it->count; i++) {
		vel->x = keys[SAPP_KEYCODE_A] - keys[SAPP_KEYCODE_D];
		vel->y = keys[SAPP_KEYCODE_LEFT_CONTROL] - keys[SAPP_KEYCODE_SPACE];
		vel->z = keys[SAPP_KEYCODE_W] - keys[SAPP_KEYCODE_S];
		v3f32_mul((float *)vel, (float *)vel, k);
	}
}

void ControllerPerspective(ecs_iter_t *it)
{
	Camera *camera = ecs_field(it, Camera, 1);
	Window *window = ecs_field(it, Window, 2);
	uint8_t *keys = window->keys;
	for (int i = 0; i < it->count; i++) {
		camera->fov = keys[SAPP_KEYCODE_KP_0] ? 45 : camera->fov;
		camera->fov -= keys[SAPP_KEYCODE_KP_1];
		camera->fov += keys[SAPP_KEYCODE_KP_2];
		//sdtx_printf("FOV: %f", camera->fov);
	}
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
	ECS_IMPORT(world, Graphics);
	ECS_IMPORT(world, Cameras);
	ECS_IMPORT(world, Sg);

	ecs_plecs_from_file(app->world, "config/graphics_attributes.flecs");
	ecs_plecs_from_file(app->world, "config/graphics_pipes.flecs");
	ecs_plecs_from_file(app->world, "config/graphics_shaders.flecs");
	ecs_plecs_from_file(app->world, "config/graphics_ubs.flecs");
	ecs_plecs_from_file(app->world, "config/app.flecs");

	// https://www.flecs.dev/explorer/?remote=true
	ecs_set(world, EcsWorld, EcsRest, {.port = 0});

	ECS_SYSTEM(world, ControllerRotate, EcsOnUpdate, Camera, Rotate3, Window($));
	ECS_SYSTEM(world, ControllerMove, EcsOnUpdate, Camera, Velocity3, Window($));
	ECS_SYSTEM(world, ControllerPerspective, EcsOnUpdate, Camera, Window($));

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
