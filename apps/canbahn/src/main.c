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
#include <egimgui.h>
#include <egspatials.h>
#include <egshapes.h>
#include <egcolors.h>
#include <egifaces.h>

#include "GuiCan.h"
#include "gui_canids.h"
#include "gui_signals.h"
#include "gui_interfaces.h"
#include "gui_exporter.h"
#include "imgui_font.h"
#include "gui_plot.h"

typedef struct {
	ecs_world_t *world;
	ecs_query_t *query_canids;
	ecs_query_t *query_signals;
	ecs_query_t *query_gui;
	ecs_query_t *query_ifaces;
	ecs_query_t *query_plots;
	ecs_query_t *query_exporter;
	ImFont *font;
	double gui_time_seconds;
	uint64_t last_time;
	sg_pass_action pass_action;

	bool show_window_main;
	bool show_window_export;
	bool show_window_extra1;
	bool show_window_extra2;

	gui_exporter_t exporter;
} app_t;

static void ShowExampleAppLog1(app_t *app)
{
	igBegin("extr1", &app->show_window_extra1, 0);
	if (igSmallButton("[Debug] Add 5 entries")) {
	}
	igEnd();
}

static void ShowExampleAppLog2(app_t *app)
{
	igBegin("extr2", &app->show_window_extra2, 0);
	if (igSmallButton("[Debug] Add 5 entries")) {
	}
	igEnd();
}


static void gui_window_main(app_t *app)
{
	ImGuiViewport *viewport = igGetMainViewport();
	igSetNextWindowPos(viewport->Pos, 0, (ImVec2){0, 0});
	igSetNextWindowSize(viewport->Size, 0);
	//igSetNextWindowViewport(viewport->ID);

	ImGuiWindowFlags flags1 = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
	igBegin("Signal window", &app->show_window_main, flags1);

	if (igBeginMenuBar()) {
		char buf[64];
		snprintf(buf, 64, "%05u µs", (uint32_t)(app->gui_time_seconds * 1000.0 * 1000.0));
		if (igBeginMenu(buf, true)) {
			igEndMenu();
		}
		if (igBeginMenu("Export", true)) {
			if (igMenuItem_Bool("Export C", NULL, false, true)) {
				app->show_window_export = true;
			}
			igMenuItem_Bool("Export Python", NULL, false, true);
			igEndMenu();
		}
		if (igBeginMenu("Extra", true)) {
			igMenuItem_Bool("Item1", NULL, false, true);
			igMenuItem_Bool("Item2", NULL, false, true);
			igSeparatorText("Separator");
			igMenuItem_Bool("Item3", NULL, false, true);
			igMenuItem_Bool("Item4", NULL, false, true);
			igMenuItem_Bool("Item5", NULL, false, true);
			igMenuItem_Bool("Item6", NULL, false, true);
			igEndMenu();
		}
		igEndMenuBar();
	}

	if (igBeginTabBar("tabs", 0)) {
		// igText("Hello");
		if (igBeginTabItem("Ifaces", NULL, 0)) {
			gui_interfaces_progress(app->world, app->query_ifaces);
			igEndTabItem();
		}
		if (igBeginTabItem("CANids", NULL, 0)) {
			gui_canids_progress(app->world, app->query_canids);
			igEndTabItem();
		}
		if (igBeginTabItem("Signals", NULL, 0)) {
			gui_signals_progress(app->world, app->query_signals);
			igEndTabItem();
		}
		if (igBeginTabItem("CustomGUI", NULL, 0)) {
			egimgui_progress1(app->world, app->query_gui);
			igEndTabItem();
		}
		if (igBeginTabItem("Plots", NULL, 0)) {
			gui_plot_progress(app->world, app->query_plots);
			igEndTabItem();
		}
		if (igBeginTabItem("Hej!", NULL, 0)) {
			igEndTabItem();
		}
		igEndTabBar();
	}
	igEnd();
}

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
		ImFontAtlas_AddFontFromFileTTF(io->Fonts, "font/roboto.ttf", 20, &config, NULL);
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
	app->pass_action.colors[0].load_action = SG_LOADACTION_CLEAR;
	app->pass_action.colors[0].clear_value = (sg_color){0.1f, 0.2f, 0.0f, 1.0f};


	app->show_window_main = true;
	app->show_window_export = false;
	app->show_window_extra1 = false;
	app->show_window_extra2 = false;
	snprintf(app->exporter.prefix, 128, "%s", "CANID");
	snprintf(app->exporter.export_destination_c, 128, "%s", "./canids_export.h");
	snprintf(app->exporter.export_destination_python, 128, "%s", "./canids_export.py");
}

void frame(app_t *app)
{
	assert(app);
	const int width = sapp_width();
	const int height = sapp_height();
	simgui_new_frame(&(simgui_frame_desc_t){
	.width = width,
	.height = height,
	.delta_time = sapp_frame_duration(),
	.dpi_scale = sapp_dpi_scale()});

	ecs_time_t gui_time_sec;
	ecs_time_measure(&gui_time_sec);

	if (app->show_window_extra1) {
		ShowExampleAppLog1(app);
	}

	if (app->show_window_extra2) {
		ShowExampleAppLog2(app);
	}

	if (app->show_window_export) {
		gui_exporter_progress(app->world, app->query_exporter, &app->exporter, &app->show_window_export);
	}

	app->show_window_main = !app->show_window_export;
	if (app->show_window_main) {
		gui_window_main(app);
	}
	


	app->gui_time_seconds = (app->gui_time_seconds * 0.99) + (ecs_time_measure(&gui_time_sec) * 0.01);

	ecs_progress(app->world, 0.0f);

	// the sokol_gfx draw pass
	sg_begin_pass(&(sg_pass){.action = app->pass_action, .swapchain = sglue_swapchain()});
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
	ECS_IMPORT(app->world, EgCan);
	ECS_IMPORT(app->world, GuiCan);
	ECS_IMPORT(app->world, EgQuantities);
	ECS_IMPORT(app->world, EgSpatials);
	ECS_IMPORT(app->world, EgShapes);
	ECS_IMPORT(app->world, EgImgui);
	ECS_IMPORT(app->world, EgColors);
	ECS_IMPORT(app->world, EgIfaces);

	ecs_log_set_level(0);
	ecs_plecs_from_dir(app->world, "config");
	/*
	ecs_plecs_from_file(app->world, "config/assembly_rc.flecs");
	ecs_plecs_from_file(app->world, "config/assembly_vision.flecs");
	//ecs_plecs_from_file(app->world, "config/assembly_bms.flecs");
	ecs_plecs_from_file(app->world, "config/assembly_movements.flecs");
	ecs_plecs_from_file(app->world, "config/assembly_odrives.flecs");
	ecs_plecs_from_file(app->world, "config/signals.flecs");
	ecs_plecs_from_file(app->world, "config/gui.flecs");
	*/
	ecs_log_set_level(-1);

	app->query_canids = gui_canids_query(app->world);
	app->query_signals = gui_signals_query(app->world);
	app->query_ifaces = gui_interfaces_query(app->world);
	app->query_gui = egimgui_query1(app->world);
	app->query_plots = gui_plot_query(app->world);
	app->query_exporter = gui_exporter_query(app->world);

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
