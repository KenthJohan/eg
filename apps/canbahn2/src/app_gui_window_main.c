#include "app_gui_window_main.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <egimgui.h>

#include "dbcsig.h"
#include "ig.h"





/*
https://www.csselectronics.com/pages/dbc-editor-can-bus-database
*/
static void show_table1(dbcsig_meta_t metas[], int metas_length, int message_length)
{
	ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_ContextMenuInBody | ImGuiTableFlags_NoHostExtendX;
	if (igBeginTable("Message", 9, flags, (ImVec2){(0.0F), (0.0F)}, (0.0F)) == false) {
		return;
	}

	igTableNextRow(0, 0);
	for (int column = 0; column < 8; column++) {
		igTableSetColumnIndex(column + 1);
		igText("b%i", column);
		igTableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(50, 50, 50, 255), -1);
	}


	for(int i = 0; i < metas_length; ++i) {
		metas[i].hover2 = false;
	}

	int bitpos = 0;
	for (int row = 0; row < message_length; row++) {
		igTableNextRow(0, 0);

		igPushID_Int(row);

		// Fill cells
		igTableSetColumnIndex(0);
		igText("%02X", row);
		igTableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(50, 50, 50, 255), -1);
		
		for (int column = 0; column < 8; column++) {
			igPushID_Int(column);
			igTableSetColumnIndex(column + 1);
			// igText("%c%c", 'A' + row, '0' + column);
			int sigint = dbcsig_meta_bitpos_to_signal(metas, metas_length, bitpos);

			char buf[128];
			snprintf(buf, 128, "%i", sigint);
			igSelectable_Bool(buf, false, 0, (ImVec2){0, 0});

			

			bitpos++;
			// Change background of Cells B1->C2
			// Demonstrate setting a cell background color with 'ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ...)'
			// (the CellBg color will be blended over the RowBg and ColumnBg colors)
			// We can also pass a column number as a third parameter to TableSetBgColor() and do this outside the column loop.
			//if (row >= 1 && row <= 2 && column >= 1 && column <= 2) {
			//	ImU32 cell_bg_color = igGetColorU32_Vec4((ImVec4){0.3f, 0.3f, 0.7f, 0.65f});
			//	igTableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color, -1);
			//}
			if (sigint >= 0) {
				if (metas[sigint].hover1) {
					igTableSetBgColor(ImGuiTableBgTarget_CellBg, hash32_to_color32(sigint, 255), -1);
				} else {
					igTableSetBgColor(ImGuiTableBgTarget_CellBg, hash32_to_color32(sigint, 200), -1);
				}
				metas[sigint].hover2 |= igIsItemHovered(0);
				if (metas[sigint].hover2) {
					//printf("sigint %i!\n", sigint);
				}
			}


			igPopID();
		}
		igPopID();
	}
	igEndTable();
}


static void show_table2(dbcsig_meta_t metas[], int metas_length)
{
	ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
	if (igBeginTable("Signals", 12, flags, (ImVec2){0, 0}, 0) == false) {
		return;
	}
	// igTableSetupColumn("bus", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultHide, 50, 0);
	// igTableSetupColumn("sock", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultHide, 50, 0);
	igTableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 20, 0);
	igTableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 100, 0);
	igTableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 100, 0);
	igTableSetupColumn("Order", ImGuiTableColumnFlags_WidthFixed, 100, 0);
	igTableSetupColumn("Mode", ImGuiTableColumnFlags_WidthFixed, 100, 0);
	igTableSetupColumn("Start", ImGuiTableColumnFlags_WidthFixed, 100, 0);
	igTableSetupColumn("Length", ImGuiTableColumnFlags_WidthFixed, 100, 0);
	igTableSetupColumn("Factor", ImGuiTableColumnFlags_WidthFixed, 100, 0);
	igTableSetupColumn("Offset", ImGuiTableColumnFlags_WidthFixed, 100, 0);
	igTableSetupColumn("Min", ImGuiTableColumnFlags_WidthFixed, 100, 0);
	igTableSetupColumn("Max", ImGuiTableColumnFlags_WidthFixed, 100, 0);
	igTableSetupColumn("Unit", ImGuiTableColumnFlags_WidthFixed, 100, 0);
	igTableHeadersRow();
	static char buf[128];
	static int start;
	static float factor;
	float row_min_height = 24.0f;


	for (int i = 0; i < metas_length; ++i) {
		dbcsig_meta_t * meta = metas + i;

		igTableNextRow(ImGuiTableRowFlags_None, row_min_height);
		igPushID_Int(i);



		igTableNextColumn();
		igTableSetBgColor(ImGuiTableBgTarget_CellBg, hash32_to_color32(i, 255), -1);
		generic_gui(&(generic_gui_t){
		.label = "##Select",
		.kind = GENERIC_GUI_KIND_TEXT_SELECTABLE_INT,
		.selectable_int.height = row_min_height,
		.selectable_int.value = i,
		.selectable_int.selected = &meta->selected});

		meta->hover1 = igIsItemHovered(0);

		if (meta->hover1) {
			igTableSetBgColor(ImGuiTableBgTarget_RowBg1, hash32_to_color32(i, 255), -1);
		}
		if (meta->hover2) {
			igTableSetBgColor(ImGuiTableBgTarget_RowBg1, hash32_to_color32(i, 255), -1);
		}

		igTableNextColumn();
		generic_gui(&(generic_gui_t){
		.label = "##Name",
		.id = i,
		.kind = GENERIC_GUI_KIND_INPUT_TEXT,
		.input.data = meta->name,
		.input.data_size = 128,
		});

		igTableNextColumn();
		igText("Type");

		igTableNextColumn();
		igText("Order");

		igTableNextColumn();
		igText("Mode");

		igTableNextColumn();
		generic_gui(&(generic_gui_t){
		.label = "##Start",
		.id = i,
		.kind = GENERIC_GUI_KIND_INPUT_INT,
		.input.data = &meta->start,
		.input.data_size = sizeof(meta->start),
		});

		igTableNextColumn();
		generic_gui(&(generic_gui_t){
		.label = "##Length",
		.id = i,
		.kind = GENERIC_GUI_KIND_INPUT_INT,
		.input.data = &meta->length,
		.input.data_size = sizeof(meta->length),
		});

		igTableNextColumn();
		generic_gui(&(generic_gui_t){
		.label = "##Factor",
		.id = i,
		.kind = GENERIC_GUI_KIND_INPUT_DOUBLE,
		.input.data = &meta->factor,
		.input.data_size = sizeof(meta->factor),
		});

		igTableNextColumn();
		generic_gui(&(generic_gui_t){
		.label = "##Offset",
		.id = i,
		.kind = GENERIC_GUI_KIND_INPUT_DOUBLE,
		.input.data = &meta->offset,
		.input.data_size = sizeof(meta->offset),
		});

		igTableNextColumn();
		generic_gui(&(generic_gui_t){
		.label = "##Min",
		.id = i,
		.kind = GENERIC_GUI_KIND_INPUT_FLOAT,
		.input.data = &meta->min,
		.input.data_size = sizeof(meta->min),
		});

		igTableNextColumn();
		generic_gui(&(generic_gui_t){
		.label = "##Max",
		.id = i,
		.kind = GENERIC_GUI_KIND_INPUT_FLOAT,
		.input.data = &meta->max,
		.input.data_size = sizeof(meta->max),
		});

		igTableNextColumn();
		igText("hej11");


		igPopID();
	}

	igEndTable();

	// ig_debug_draw();
}

void app_gui_window_main_init(app_t *app)
{
}

void app_gui_window_main(app_t *app)
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
			#define METAS_COUNT 10
			static dbcsig_meta_t metas[METAS_COUNT] = {
			{.name = "WheelBased", .type = 0, .order = 0, .mode = 0, .start = 0, .length = 16, .factor = 0.01, .offset = 0, .min = 0, .max = 2500, .unit = "km/h"},
			{.name = "EngineSpeed", .type = 0, .order = 0, .mode = 0, .start = 24, .length = 16, .factor = 0.125, .offset = 0, .min = 0, .max = 2500, .unit = "rpm"}};

			show_table1(metas, METAS_COUNT, 64);
			igSameLine(0, 10);
			show_table2(metas, METAS_COUNT);
			igEndTabItem();
		}
		if (igBeginTabItem("Tab2", NULL, 0)) {
			igEndTabItem();
		}

		igEndTabBar();
	}
	igEnd();
}