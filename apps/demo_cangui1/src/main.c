#include <stdlib.h>
#include <flecs.h>

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
#include "gui_signals.h"
#include "gui_interfaces.h"
#include "imgui_font.h"
#include "gui_plot.h"

typedef struct {
	ecs_world_t *world;
	ecs_query_t *query_signals;
	ecs_query_t *query_gui;
	ecs_query_t *query_ifaces;
	ecs_query_t *query_plots;
	ImFont *font;
} app_t;

typedef struct {
	uint64_t last_time;
	bool show_test_window;
	bool show_another_window;
	sg_pass_action pass_action;
} state_t;
static state_t state;

void init(void *user)
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
		//ImFontAtlas_AddFontFromFileTTF(io->Fonts, "/usr/share/fonts/truetype/ubuntu/UbuntuMono[wght].ttf", 20, &config, NULL);
	    ImFontAtlas_AddFontFromFileTTF(io->Fonts, "font/roboto.ttf", 20, &config, NULL);
		//ImFontAtlas_AddFontFromMemoryTTF(io->Fonts, dump_font, sizeof(dump_font), 16.0f, &config, NULL);

		// create font texture and linear-filtering sampler for the custom font
		// NOTE: linear filtering looks better on low-dpi displays, while
		// nearest-filtering looks better on high-dpi displays
		simgui_font_tex_desc_t font_texture_desc = {0};
		font_texture_desc.min_filter = SG_FILTER_LINEAR;
		font_texture_desc.mag_filter = SG_FILTER_LINEAR;
		simgui_create_fonts_texture(&font_texture_desc);
	}

	/* initialize application state */
	state = (state_t){
	.show_test_window = true,
	.pass_action = {
	.colors[0] = {
	.load_action = SG_LOADACTION_CLEAR,
	.clear_value = {0.7f, 0.5f, 0.0f, 1.0f}}}};
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

	// 1. Show a simple window
	// Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
	/*
	static float f = 0.0f;
	igText("Hello, world!");
	igSliderFloat("float", &f, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_None);
	igColorEdit3("clear color", (float*)&state.pass_action.colors[0].clear_value, 0);
	if (igButton("Test Window", (ImVec2) { 0.0f, 0.0f})) state.show_test_window ^= 1;
	if (igButton("Another Window", (ImVec2) { 0.0f, 0.0f })) state.show_another_window ^= 1;
	igText("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / igGetIO()->Framerate, igGetIO()->Framerate);
	*/

	// 2. Show another simple window, this time using an explicit Begin/End pair
	if (state.show_another_window || 1) {
		ImGuiViewport *viewport = igGetMainViewport();
		igSetNextWindowPos(viewport->Pos, 0, (ImVec2){0, 0});
		igSetNextWindowSize(viewport->Size, 0);
		// igSetNextWindowViewport(viewport->ID);
		ImGuiWindowFlags flags1 = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
		igBegin("Signal window", &state.show_another_window, flags1);
		igBeginTabBar("tabs", 0);
		// igText("Hello");
		if (igBeginTabItem("Ifaces", NULL, 0)) {
			gui_interfaces_progress(app->world, app->query_ifaces);
			igEndTabItem();
		};
		if (igBeginTabItem("Signals", NULL, 0)) {
			gui_signals_progress(app->world, app->query_signals);
			igEndTabItem();
		};
		if (igBeginTabItem("CustomGUI", NULL, 0)) {
			egimgui_progress1(app->world, app->query_gui);
			igEndTabItem();
		};
		if (igBeginTabItem("Plots", NULL, 0)) {
			gui_plot_progress(app->world, app->query_plots);
			igEndTabItem();
		};
		igEndTabBar();
		igEnd();
	}

	ecs_progress(app->world, 0.0f);

	// the sokol_gfx draw pass
	sg_begin_pass(&(sg_pass){.action = state.pass_action, .swapchain = sglue_swapchain()});
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
	app_t *app = malloc(sizeof(app_t));
	app->world = ecs_init();
	ECS_IMPORT(app->world, FlecsUnits);
	ECS_IMPORT(app->world, EgCan);
	ECS_IMPORT(app->world, GuiCan);
	ECS_IMPORT(app->world, EgQuantities);
	ECS_IMPORT(app->world, EgSpatials);
	ECS_IMPORT(app->world, EgShapes);
	ECS_IMPORT(app->world, EgImgui);
	ECS_IMPORT(app->world, EgColors);
	ECS_IMPORT(app->world, EgIfaces);

	ecs_log_set_level(1);
	ecs_plecs_from_file(app->world, "config/signals.flecs");
	ecs_plecs_from_file(app->world, "config/gui.flecs");
	ecs_log_set_level(-1);

	app->query_signals = gui_signals_query(app->world);
	app->query_ifaces = gui_interfaces_query(app->world);
	app->query_gui = egimgui_query1(app->world);
	app->query_plots = gui_plot_query(app->world);

	// https://www.flecs.dev/explorer/?remote=true
	ecs_set(app->world, EcsWorld, EcsRest, {.port = 0});
	printf("Remote: %s\n", "https://www.flecs.dev/explorer/?remote=true");


    sapp_desc desc = { };
    desc.init_userdata_cb = init,
    desc.frame_userdata_cb = (void (*)(void *))frame,
    desc.cleanup_userdata_cb = cleanup,
    desc.event_userdata_cb = input,
	desc.user_data = app,
    desc.width = 1400;
    desc.height = 800;
    desc.fullscreen = false;
    desc.high_dpi = true;
    //desc.html5_ask_leave_site = html5_ask_leave_site;
    desc.ios_keyboard_resizes_canvas = false;
    desc.window_title = "Dear ImGui HighDPI (sokol-app)";
    desc.icon.sokol_default = true;
    desc.enable_clipboard = true;
    desc.logger.func = slog_func;
    return desc;
}
