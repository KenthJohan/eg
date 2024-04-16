//------------------------------------------------------------------------------
//  cimgui-sapp.c
//
//  Demonstrates Dear ImGui UI rendering in C via
//  sokol_gfx.h + sokol_imgui.h + cimgui.h
//------------------------------------------------------------------------------
#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_log.h>
#include <sokol_glue.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <sokol_imgui.h>
#include <flecs.h>
#include <stdlib.h>
#include <assert.h>
#include <egcan.h>
#include <egquantities.h>
#include "GuiCan.h"

typedef struct {
	ecs_world_t * world;
	ecs_query_t * q1;
	ecs_query_t * q2;
} app_t;


typedef struct {
    uint64_t last_time;
    bool show_test_window;
    bool show_another_window;
    sg_pass_action pass_action;
} state_t;
static state_t state;

void init(void * user) {
    // setup sokol-gfx, sokol-time and sokol-imgui
    sg_setup(&(sg_desc){
        .environment = sglue_environment(),
        .logger.func = slog_func,
    });

    // use sokol-imgui with all default-options (we're not doing
    // multi-sampled rendering or using non-default pixel formats)
    simgui_setup(&(simgui_desc_t){
        .logger.func = slog_func,
    });

    /* initialize application state */
    state = (state_t) {
        .show_test_window = true,
        .pass_action = {
            .colors[0] = {
                .load_action = SG_LOADACTION_CLEAR,
                .clear_value = { 0.7f, 0.5f, 0.0f, 1.0f }
            }
        }
    };
}

void frame(app_t * app) {
	assert(app);
    const int width = sapp_width();
    const int height = sapp_height();
    simgui_new_frame(&(simgui_frame_desc_t){
        .width = width,
        .height = height,
        .delta_time = sapp_frame_duration(),
        .dpi_scale = sapp_dpi_scale()
    });

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
        igSetNextWindowSize((ImVec2){600,400}, ImGuiCond_FirstUseEver);
        igBegin("Signal window", &state.show_another_window, 0);
        //igText("Hello");
		gui_can_progress1(app->world, app->q1);
        igEnd();
    }

	ecs_progress(app->world, 0.0f);

    // the sokol_gfx draw pass
    sg_begin_pass(&(sg_pass){ .action = state.pass_action, .swapchain = sglue_swapchain() });
    simgui_render();
    sg_end_pass();
    sg_commit();
}

void cleanup(void * user) {
    simgui_shutdown();
    sg_shutdown();
}

void input(const sapp_event* event, void * user) {
    simgui_handle_event(event);
}





sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
	app_t * app = malloc(sizeof(app_t));
	app->world = ecs_init();
	ECS_IMPORT(app->world, FlecsUnits);
	ECS_IMPORT(app->world, EgCan);
	ECS_IMPORT(app->world, GuiCan);
	ECS_IMPORT(app->world, EgQuantities);
	// https://www.flecs.dev/explorer/?remote=true
	ecs_set(app->world, EcsWorld, EcsRest, {.port = 0});

	ecs_log_set_level(1);
	ecs_plecs_from_file(app->world, "config/signals.flecs");
	ecs_log_set_level(-1);

	// clang-format off
	app->q1 = ecs_query(app->world, {
		.filter.terms = {
			{.id = ecs_id(GuiSlider)},
			{.id = ecs_id(EgCanSignal)},
			{.id = ecs_id(EgQuantitiesIsq), .oper = EcsOptional}
		}
		}
	);
	app->q2 = ecs_query(app->world, {
		.filter.terms = {
			{.id = ecs_id(GuiCanSignalInfo)}
		}
		}
	);
	// clang-format on


    return (sapp_desc) {
        .init_userdata_cb = init,
        .frame_userdata_cb = (void (*)(void*))frame,
        .cleanup_userdata_cb = cleanup,
        .event_userdata_cb = input,
		.user_data = app,
        .width = 1024,
        .height = 768,
        .window_title = "cimgui (sokol-app)",
        .ios_keyboard_resizes_canvas = false,
        .icon.sokol_default = true,
        .enable_clipboard = true,
        .logger.func = slog_func,
    };
}
