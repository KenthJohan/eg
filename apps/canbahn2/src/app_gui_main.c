#include "app_gui_main.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <egimgui.h>

#include "dbcsig.h"
#include "ig.h"
#include "app_gui_cansig.h"

void app_gui_main(app_t *app)
{
	ImGuiViewport *viewport = igGetMainViewport();
	igSetNextWindowPos(viewport->Pos, 0, (ImVec2){0, 0});
	igSetNextWindowSize(viewport->Size, 0);
	// igSetNextWindowViewport(viewport->ID);

	ImGuiWindowFlags_ flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
	igBegin("Signal window", &app->show_window_main, flags);

	if (igBeginMenuBar()) {
		char buf[64];
		snprintf(buf, 64, "%05u µs", (uint32_t)(app->gui_time_seconds * 1000.0 * 1000.0));
		if (igBeginMenu(buf, true)) {
			igEndMenu();
		}
		if (igBeginMenu("Extra", true)) {
			app->show_window_extra1 = igMenuItem_Bool("extra1", NULL, false, true);
			app->show_window_extra2 = igMenuItem_Bool("extra2", NULL, false, true);
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
		if (igBeginTabItem("Message", NULL, 0)) {
			#define CANSIG_COUNT 10
			static app_gui_cansig_state_t guistates[CANSIG_COUNT] = {};
			static dbcsig_meta_t metas[CANSIG_COUNT] = {
			{.name = "WheelBased", .type = 0, .order = 0, .mode = 0, .start = 0, .length = 16, .factor = 0.01, .offset = 0, .min = 0, .max = 2500, .unit = "km/h"},
			{.name = "EngineSpeed", .type = 0, .order = 0, .mode = 0, .start = 24, .length = 16, .factor = 0.125, .offset = 0, .min = 0, .max = 2500, .unit = "rpm"},
			{.name = "WindSpeed1", .type = 0, .order = 0, .mode = 0, .start = 16, .length = 4, .factor = 0.125, .offset = 0, .min = 0, .max = 2500, .unit = "km/h"},
			{.name = "WindSpeed2", .type = 0, .order = 0, .mode = 0, .start = 20, .length = 4, .factor = 0.125, .offset = 0, .min = 0, .max = 2500, .unit = "km/h"}};

			app_gui_cansig_table1(guistates, metas, CANSIG_COUNT, 64);
			igSameLine(0, 10);
			app_gui_cansig_table2(guistates, metas, CANSIG_COUNT);
			igEndTabItem();
		}
		if (igBeginTabItem("Tab2", NULL, 0)) {
			igEndTabItem();
		}

		igEndTabBar();
	}
	igEnd();
}