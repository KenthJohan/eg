#include <gs/gs.h>
#include <gs/util/gs_idraw.h>
#include <flecs.h>
#include "EgQuantities.h"
#include "EgGui.h"
#include "EgUserinput.h"
#include "GsDraw.h"
#include "GsUserinput.h"

typedef struct app_t
{
	ecs_world_t *world;
	gs_command_buffer_t cb;
	gs_immediate_draw_t gsi;
} app_t;


void app_init()
{
	app_t *app = gs_user_data(app_t);
	app->world = ecs_init();
	app->cb = gs_command_buffer_new();
	app->gsi = gs_immediate_draw_new(gs_platform_main_window());

	ECS_IMPORT(app->world, EgQuantities);
	ECS_IMPORT(app->world, EgGui);
	ECS_IMPORT(app->world, EgUserinput);
	ECS_IMPORT(app->world, GsDraw);
	ECS_IMPORT(app->world, GsUserinput);



	// https://www.flecs.dev/explorer/?remote=true
	ecs_set(app->world, EcsWorld, EcsRest, {.port = 0});

	// Setup a singleton for mouse position:
	ecs_add_id(app->world, ecs_id(EgMouse), ecs_pair(ecs_id(EgV2F32), EgPosition));
	ecs_add_id(app->world, ecs_id(EgMouse), ecs_pair(ecs_id(EgV2F32), EgVelocity));
	ecs_add_id(app->world, ecs_id(EgMouse), ecs_id(EgMouse));
	ecs_add_id(app->world, ecs_id(EgHover), ecs_id(EgHover));
	ecs_add_id(app->world, ecs_id(EgHover), ecs_pair(ecs_id(EgV2F32), EgPositionRelative));
	ecs_add_id(app->world, ecs_id(EgGuiDrag), ecs_id(EgGuiDrag));
	ecs_add_id(app->world, ecs_id(EgGuiDrag), ecs_pair(ecs_id(EgV2F32), EgPositionRelative));
	ecs_add_id(app->world, ecs_id(EgKeyboard), ecs_id(EgKeyboard));
	ecs_plecs_from_file(app->world, "test.flecs");

	ecs_entity_t e_draw = ecs_lookup(app->world, "Draw");
	ecs_set(app->world, e_draw, GsImmediateDraw, {&app->gsi});
    //ecs_override(app->world, e_draw, EgZIndex);

}

void app_update()
{
	app_t *app = gs_user_data(app_t);

	gs_command_buffer_t *cb = &app->cb;
	gs_immediate_draw_t *gsi = &app->gsi;

	gs_vec2 fbs = gs_platform_framebuffer_sizev(gs_platform_main_window());
	const float t = gs_platform_elapsed_time() * 0.0001f;

	if (0)
	{
		gsi_camera3D(gsi, fbs.x, fbs.y);
		gsi_rotatev(gsi, gs_deg2rad(90.f), GS_ZAXIS);
		gsi_rotatev(gsi, t, GS_YAXIS);
		gsi_sphere(gsi, 0.f, 0.f, 0.f, 1.f, 50, 150, 200, 50, GS_GRAPHICS_PRIMITIVE_LINES);
	}

	gsi_camera2D(gsi, fbs.x, fbs.y);
	
	ecs_progress(app->world, 0);
	gsi_text(gsi, fbs.x * 0.5f - 198.f, fbs.y * 0.5f, "ESC to open term, TAB toggle autoscroll, help for more", NULL, false, 255, 255, 255, 255);
	gsi_renderpass_submit(gsi, cb, gs_v4(0.f, 0.f, fbs.x, fbs.y), gs_color(10, 10, 10, 255));


	// Submit command buffer for GPU
	gs_graphics_command_buffer_submit(cb);
}

void app_shutdown()
{
	app_t *app = gs_user_data(app_t);
	gs_immediate_draw_free(&app->gsi);
	gs_command_buffer_free(&app->cb);
}

gs_app_desc_t gs_main(int32_t argc, char **argv)
{
	app_t *app = gs_malloc_init(app_t);



	return (gs_app_desc_t){
		.user_data = app,
		.window.width = 800,
		.window.height = 600,
		.init = app_init,
		.update = app_update,
		.shutdown = app_shutdown};
}
