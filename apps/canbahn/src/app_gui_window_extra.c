#include "app_gui_window_extra.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>

void app_gui_window_extra1(app_t *app)
{
	igSetNextWindowPos((ImVec2){100, 100}, ImGuiCond_Once, (ImVec2){0, 0});
	igSetNextWindowSize((ImVec2){400, 300}, ImGuiCond_Once);
	ImGuiWindowFlags_ flags = 0;
	igBegin("Extra1", &app->show_window_extra1, flags);
	if (igSmallButton("Button")) {
	}
	igEnd();
}

void app_gui_window_extra2(app_t *app)
{
	igSetNextWindowPos((ImVec2){100, 100}, ImGuiCond_Once, (ImVec2){0, 0});
	igSetNextWindowSize((ImVec2){400, 300}, ImGuiCond_Once);
	ImGuiWindowFlags_ flags = 0;
	igBegin("Extra2", &app->show_window_extra2, flags);
	if (igSmallButton("Button")) {
	}
	igEnd();
}
