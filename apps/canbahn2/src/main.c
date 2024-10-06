#include <stdlib.h>

#include <flecs.h>
#include <flecs_extra.h>

#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_log.h>
#include <sokol_glue.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <sokol_imgui.h>

#include <egcan.h>
#include <egquantities.h>
#include <egspatials.h>
#include <egshapes.h>
#include <egcolors.h>
#include <egifaces.h>
#include <egfs.h>
#include <egstr.h>
#include <egimgui.h>


#include "imgui_font.h"
#include "app.h"
#include "app_gui_main.h"

void init(app_t *app)
{
	// setup sokol-gfx, sokol-time and sokol-imgui
	sg_desc desc = {0};
	desc.environment = sglue_environment();
	desc.logger.func = slog_func;
	sg_setup(&desc);

	// use sokol-imgui with all default-options (we're not doing
	// multi-sampled rendering or using non-default pixel formats)
	/*
	simgui_setup(&(simgui_desc_t){
	    .logger.func = slog_func,
	    .write_alpha_channel = true
	});
	*/

	{
		simgui_desc_t simgui_desc = {0};
		simgui_desc.no_default_font = true;
		simgui_desc.logger.func = slog_func;
		simgui_setup(&simgui_desc);

		// configure Dear ImGui with our own embedded font
		struct ImGuiIO *io = igGetIO();
		/*
		ImFontConfig fontCfg = {};
		fontCfg.FontDataOwnedByAtlas = false;
		fontCfg.OversampleH = 2;
		fontCfg.OversampleV = 2;
		fontCfg.RasterizerMultiply = 1.5f;
		*/
		ImFontConfig config = *ImFontConfig_ImFontConfig();
		// ImFontAtlas_AddFontFromFileTTF(io->Fonts, "/usr/share/fonts/truetype/ubuntu/UbuntuMono[wght].ttf", 20, &config, NULL);
		ImFontAtlas_AddFontFromFileTTF(io->Fonts, "font/roboto.ttf", 18, &config, NULL);
		// ImFontAtlas_AddFontFromMemoryTTF(io->Fonts, dump_font, sizeof(dump_font), 16.0f, &config, NULL);

		// create font texture and linear-filtering sampler for the custom font
		// NOTE: linear filtering looks better on low-dpi displays, while
		// nearest-filtering looks better on high-dpi displays
		simgui_font_tex_desc_t font_texture_desc = {0};
		font_texture_desc.min_filter = SG_FILTER_LINEAR;
		font_texture_desc.mag_filter = SG_FILTER_LINEAR;
		simgui_create_fonts_texture(&font_texture_desc);
	}

	/* initialize application state */
	app->show_window_main = true;
	app->show_window_extra1 = false;
	app->show_window_extra2 = false;

	sg_sampler smp_nearest = sg_make_sampler(&(sg_sampler_desc){
	.min_filter = SG_FILTER_NEAREST,
	.mag_filter = SG_FILTER_NEAREST,
	});

	app->image.width = 200;
	app->image.height = 200;
	app->image.depth = 4;
	app->image.size = app->image.width * app->image.height * app->image.depth;
	app->image.data = malloc(app->image.size);
	sg_image render_depth_img = sg_make_image(&(sg_image_desc){
	.usage = SG_USAGE_STREAM,
	.width = app->image.width,
	.height = app->image.height,
	.data.subimage[0][0].ptr = 0,
	.data.subimage[0][0].size = 0,
	.pixel_format = SG_PIXELFORMAT_RGBA8,
	});

	simgui_image_t simgui_img = simgui_make_image(&(simgui_image_desc_t){
	.image = render_depth_img,
	.sampler = smp_nearest,
	});

	app->image_id = simgui_img.id;
	app->image_id2 = render_depth_img.id;
}

void frame(app_t *app)
{
	assert(app);
	sg_pass pass = {0};
	pass.action.colors[0].load_action = SG_LOADACTION_CLEAR;
	pass.action.colors[0].clear_value = (sg_color){0.1f, 0.2f, 0.0f, 1.0f};
	pass.swapchain = sglue_swapchain();

	simgui_new_frame(&(simgui_frame_desc_t){
	.width = pass.swapchain.width,
	.height = pass.swapchain.height,
	.delta_time = sapp_frame_duration(),
	.dpi_scale = sapp_dpi_scale()});

	ecs_time_t gui_time_sec;
	ecs_time_measure(&gui_time_sec);

	if (app->show_window_extra1) {
	}

	if (app->show_window_extra2) {
	}

	if (app->show_window_main) {
		app_gui_main(app);
	}

	app->gui_time_seconds = (app->gui_time_seconds * 0.99) + (ecs_time_measure(&gui_time_sec) * 0.01);

	ecs_progress(app->world, 0.0f);

	// the sokol_gfx draw pass

	sg_begin_pass(&pass);
	simgui_render();
	sg_end_pass();
	sg_commit();
}

void cleanup(void *user)
{
	simgui_shutdown();
	sg_shutdown();
}

void input(const sapp_event *event, void *user)
{
	simgui_handle_event(event);
}

sapp_desc sokol_main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	ecs_os_set_api_defaults();
	app_t *app = ecs_os_calloc_t(app_t);
	app->world = ecs_init();
	ECS_IMPORT(app->world, FlecsUnits);
	ECS_IMPORT(app->world, FlecsDoc);

	ecs_log_set_level(0);
	ecs_script_run_file(app->world, "config/2-signals.flecs");
	ecs_log_set_level(-1);

	// https://www.flecs.dev/explorer/?remote=true
	ecs_set(app->world, EcsWorld, EcsRest, {.port = 0});
	printf("Remote: %s\n", "https://www.flecs.dev/explorer/?remote=true");

	sapp_desc desc = {};
	desc.init_userdata_cb = (void (*)(void *))init,
	desc.frame_userdata_cb = (void (*)(void *))frame,
	desc.cleanup_userdata_cb = (void (*)(void *))cleanup,
	desc.event_userdata_cb = (void (*)(const sapp_event *, void *))input,
	desc.user_data = app,
	desc.width = 1400;
	desc.height = 800;
	desc.fullscreen = false;
	desc.high_dpi = true;
	// desc.html5_ask_leave_site = html5_ask_leave_site;
	desc.ios_keyboard_resizes_canvas = false;
	desc.window_title = "CAN-bahn GUI 2024";
	desc.icon.sokol_default = true;
	desc.enable_clipboard = true;
	desc.logger.func = slog_func;
	return desc;
}
