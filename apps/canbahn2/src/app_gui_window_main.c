#include "app_gui_window_main.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <egimgui.h>

// Helpers macros to generate 32-bits encoded colors
#ifdef IMGUI_USE_BGRA_PACKED_COLOR
#define IM_COL32_R_SHIFT 16
#define IM_COL32_G_SHIFT 8
#define IM_COL32_B_SHIFT 0
#define IM_COL32_A_SHIFT 24
#define IM_COL32_A_MASK 0xFF000000
#else
#define IM_COL32_R_SHIFT 0
#define IM_COL32_G_SHIFT 8
#define IM_COL32_B_SHIFT 16
#define IM_COL32_A_SHIFT 24
#define IM_COL32_A_MASK 0xFF000000
#endif
#define IM_COL32(R, G, B, A) (((ImU32)(A) << IM_COL32_A_SHIFT) | ((ImU32)(B) << IM_COL32_B_SHIFT) | ((ImU32)(G) << IM_COL32_G_SHIFT) | ((ImU32)(R) << IM_COL32_R_SHIFT))
#define IM_COL32_WHITE IM_COL32(255, 255, 255, 255) // Opaque white = 0xFFFFFFFF
#define IM_COL32_BLACK IM_COL32(0, 0, 0, 255)       // Opaque black
#define IM_COL32_BLACK_TRANS IM_COL32(0, 0, 0, 0)   // Transparent black = 0x00000000

static void ig_debug_draw()
{
	ImVec2 my_pos;
	igGetCursorScreenPos(&my_pos);
	if (igIsKeyDown_Nil(ImGuiKey_J)) {
		igDebugDrawItemRect(4278190335U); // Helper to draw a rectangle between GetItemRectMin() and GetItemRectMax()
		ImDrawList_AddCircleFilled(igGetForegroundDrawList_Nil(), my_pos, 3, IM_COL32(255, 0, 0, 255), 0);
	}
}

static void show_table()
{
	ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_ContextMenuInBody | ImGuiTableFlags_NoHostExtendX;
	if (igBeginTable("table1", 9, flags, (ImVec2){(0.0F), (0.0F)}, (0.0F))) {

		igTableNextRow(0, 0);
		for (int column = 0; column < 8; column++) {
			igTableSetColumnIndex(column+1);
			igText("b%i", column);
		}

		for (int row = 0; row < 9; row++) {
			igTableNextRow(0, 0);

			// Fill cells
			igTableSetColumnIndex(0);
			igText("B%i", row);
			for (int column = 0; column < 8; column++) {
				igTableSetColumnIndex(column+1);
				//igText("%c%c", 'A' + row, '0' + column);
				char buf[128];
				snprintf(buf, 128, "%c%c", 'A' + row, '0' + column);
				igSelectable_Bool(buf, false, 0, (ImVec2){0,0});

				// Change background of Cells B1->C2
				// Demonstrate setting a cell background color with 'ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ...)'
				// (the CellBg color will be blended over the RowBg and ColumnBg colors)
				// We can also pass a column number as a third parameter to TableSetBgColor() and do this outside the column loop.
				if (row >= 1 && row <= 2 && column >= 1 && column <= 2) {
					ImU32 cell_bg_color = igGetColorU32_Vec4((ImVec4){0.3f, 0.3f, 0.7f, 0.65f});
					igTableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color, -1);
				}
			}
		}
		igEndTable();
	}
	ig_debug_draw();
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
			show_table();
			igEndTabItem();
		}
		if (igBeginTabItem("Tab2", NULL, 0)) {
			igEndTabItem();
		}

		igEndTabBar();
	}
	igEnd();
}