#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_log.h>
#include <sokol_glue.h>
#include <sokol_gl.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <float.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>

#include <flecs.h>
#include <egsokol.h>

ecs_world_t *world;

static void init(void)
{
	sg_setup(&(sg_desc){
	.environment = sglue_environment(),
	.logger.func = slog_func,
	});

	world = ecs_init();

	sgl_setup(&(sgl_desc_t){});

	ECS_IMPORT(world, Sg);

	ecs_log_set_level(1);
	// ecs_script_run_file(app->world, "config/keycode_sokol.flecs");
	ecs_script_run_file(world, "config/graphics_attributes.flecs");
	ecs_script_run_file(world, "config/graphics_ubs.flecs");
	ecs_script_run_file(world, "config/graphics_shaders.flecs");
	// ecs_progress(app->world, 0);
	ecs_script_run_file(world, "config/graphics_pipes.flecs");
	/*
	ecs_progress(app->world, 0);
	ecs_progress(app->world, 0);
	ecs_progress(app->world, 0);
	*/
	// ecs_progress(app->world, 0);
	ecs_log_set_level(-1);

	// https://www.flecs.dev/explorer/?remote=true
	ecs_set(world, EcsWorld, EcsRest, {.port = 0});
	printf("https://www.flecs.dev/explorer/?remote=true\n");
}

void frame(void)
{
		ecs_progress(world, 0);
}

void cleanup(void)
{
	sg_shutdown();
}

sapp_desc sokol_main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	return (sapp_desc){
	.init_cb = init,
	.frame_cb = frame,
	.cleanup_cb = cleanup,
	.width = 640,
	.height = 480,
	.window_title = "Triangle",
	.icon.sokol_default = true,
	.logger.func = slog_func,
	};
}