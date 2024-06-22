#include "app_gui_window_main.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>


#include <egimgui.h>

#include "gui_canids.h"
#include "gui_signals.h"
#include "gui_interfaces.h"
#include "gui_exporter.h"
#include "gui_plot.h"

void app_gui_window_main_init(app_t *app)
{
	app->query_canids = gui_canids_query(app->world);
	app->query_signals = gui_signals_query(app->world);
	app->query_ifaces = gui_interfaces_query(app->world);
	app->query_gui = egimgui_query1(app->world);
	app->query_plots = gui_plot_query(app->world);
	app->query_exporter = gui_exporter_query(app->world);
	app->query_exporter2 = gui_exporter_query2(app->world);
}


void app_gui_window_main(app_t *app)
{
	ImGuiViewport *viewport = igGetMainViewport();
	igSetNextWindowPos(viewport->Pos, 0, (ImVec2){0, 0});
	igSetNextWindowSize(viewport->Size, 0);
	//igSetNextWindowViewport(viewport->ID);

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
		if (igBeginTabItem("Export", NULL, 0)) {
			gui_exporter_progress(app->world, app->query_exporter, app->query_exporter2);
			igEndTabItem();
		}
		igEndTabBar();
	}
	igEnd();
}