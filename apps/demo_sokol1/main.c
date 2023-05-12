#include "flecs.h"
#include "sokol_app.h"
#include "test.h"
#include "sokol_log.h"



sapp_desc sokol_main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;
	ecs_world_t *world = ecs_init();
	//https://www.flecs.dev/explorer/?remote=true
	ecs_set(world, EcsWorld, EcsRest, {.port = 0});
	return (sapp_desc) {
		.user_data = world,
		.init_userdata_cb = init,
		.frame_userdata_cb = frame,
		.cleanup_userdata_cb = cleanup,
		//.event_cb = __dbgui_event,
		.width = 800,
		.height = 600,
		.sample_count = 4,
		.window_title = "sokol-gl contexts (sapp)",
		.icon.sokol_default = true,
		.logger.func = slog_func,
	};

}