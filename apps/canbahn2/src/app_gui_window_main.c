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

typedef struct {
	char name[128];
	int type;
	int order;
	int mode;
	int start;
	int length;
	double factor;
	double offset;
	double min;
	double max;
	char unit[128];
} signal_table_t;

static int signal_table_bitpos_to_signal(signal_table_t table[], int length, int bitpos)
{
	for(int i = 0; i < length; ++i) {
		if (table[i].start > bitpos) {
			continue;
		}
		if ((table[i].start + table[i].length) <= bitpos) {
			continue;
		}
		return i;
	}
	return -1;
}

/*
https://www.csselectronics.com/pages/dbc-editor-can-bus-database
*/
static void show_table1(signal_table_t table[], int table_length, int message_length)
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
			char buf[128];
			snprintf(buf, 128, "%i", signal_table_bitpos_to_signal(table, table_length, bitpos));
			igSelectable_Bool(buf, false, 0, (ImVec2){0, 0});
			bitpos++;
			// Change background of Cells B1->C2
			// Demonstrate setting a cell background color with 'ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ...)'
			// (the CellBg color will be blended over the RowBg and ColumnBg colors)
			// We can also pass a column number as a third parameter to TableSetBgColor() and do this outside the column loop.
			if (row >= 1 && row <= 2 && column >= 1 && column <= 2) {
				ImU32 cell_bg_color = igGetColorU32_Vec4((ImVec4){0.3f, 0.3f, 0.7f, 0.65f});
				igTableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color, -1);
			}
			igPopID();
		}
		igPopID();
	}
	igEndTable();
}

typedef enum {
	GENERIC_GUI_KIND_TEXT_INT,
	GENERIC_GUI_KIND_TEXT_SELECTABLE,
	GENERIC_GUI_KIND_TEXT_SELECTABLE_INT,
	GENERIC_GUI_KIND_INPUT_TEXT,
	GENERIC_GUI_KIND_INPUT_INT,
	GENERIC_GUI_KIND_INPUT_FLOAT,
	GENERIC_GUI_KIND_INPUT_DOUBLE,
} generic_gui_kind_t;

typedef struct {
	int id;
	generic_gui_kind_t kind;
	char const *label;
	union {
		struct {
			void *data;
			size_t data_size;
		} input;

		struct {
			int value;
		} text_int;

		struct {
			int value;
			bool *selected;
		} selectable_int;
	};
} generic_gui_t;

static void generic_gui(generic_gui_t *item)
{
	igPushID_Int(item->id);
	switch (item->kind) {
	case GENERIC_GUI_KIND_TEXT_INT:
		igText("%i", item->text_int.value);
		break;

	case GENERIC_GUI_KIND_TEXT_SELECTABLE_INT: {
		char buf[128];
		snprintf(buf, 128, "%i", item->text_int.value);
		//*item->selectable_int.selected = igSelectable_Bool(buf, *item->selectable_int.selected, ImGuiSelectableFlags_SpanAllColumns, (ImVec2){0, 0});
	} break;

	case GENERIC_GUI_KIND_INPUT_TEXT:
		igPushItemWidth(-1);
		igInputText(item->label, item->input.data, item->input.data_size, 0, 0, 0);
		igPopItemWidth();
		break;

	case GENERIC_GUI_KIND_INPUT_INT:
		igPushItemWidth(-1);
		igInputInt(item->label, item->input.data, 0, 0, 0);
		igPopItemWidth();
		break;

	case GENERIC_GUI_KIND_INPUT_FLOAT:
		igPushItemWidth(-1);
		igInputFloat(item->label, item->input.data, 0, 0, "%f", 0);
		igPopItemWidth();
		break;

	case GENERIC_GUI_KIND_INPUT_DOUBLE:
		igPushItemWidth(-1);
		igInputDouble(item->label, item->input.data, 0, 0, "%f", 0);
		igPopItemWidth();
		break;

	default:
		break;
	}
	igPopID();
}

static void show_table2(signal_table_t value[], int length)
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
	static bool selected[64] = {false};
	for (int i = 0; i < length; ++i) {

		igTableNextColumn();
		generic_gui(&(generic_gui_t){
		.label = "##Select",
		.kind = GENERIC_GUI_KIND_TEXT_SELECTABLE_INT,
		.selectable_int.value = i,
		.selectable_int.selected = selected + i});

		igTableNextColumn();
		generic_gui(&(generic_gui_t){
		.label = "##Name",
		.id = i,
		.kind = GENERIC_GUI_KIND_INPUT_TEXT,
		.input.data = value[i].name,
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
		.input.data = &value[i].start,
		.input.data_size = sizeof(value[i].start),
		});

		igTableNextColumn();
		generic_gui(&(generic_gui_t){
		.label = "##Length",
		.id = i,
		.kind = GENERIC_GUI_KIND_INPUT_INT,
		.input.data = &value[i].length,
		.input.data_size = sizeof(value[i].length),
		});

		igTableNextColumn();
		generic_gui(&(generic_gui_t){
		.label = "##Factor",
		.id = i,
		.kind = GENERIC_GUI_KIND_INPUT_DOUBLE,
		.input.data = &value[i].factor,
		.input.data_size = sizeof(value[i].factor),
		});

		igTableNextColumn();
		generic_gui(&(generic_gui_t){
		.label = "##Offset",
		.id = i,
		.kind = GENERIC_GUI_KIND_INPUT_DOUBLE,
		.input.data = &value[i].offset,
		.input.data_size = sizeof(value[i].offset),
		});

		igTableNextColumn();
		generic_gui(&(generic_gui_t){
		.label = "##Min",
		.id = i,
		.kind = GENERIC_GUI_KIND_INPUT_FLOAT,
		.input.data = &value[i].min,
		.input.data_size = sizeof(value[i].min),
		});

		igTableNextColumn();
		generic_gui(&(generic_gui_t){
		.label = "##Max",
		.id = i,
		.kind = GENERIC_GUI_KIND_INPUT_FLOAT,
		.input.data = &value[i].max,
		.input.data_size = sizeof(value[i].max),
		});

		igTableNextColumn();
		igText("hej11");
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
			static signal_table_t stable[5] = {
			{.name = "WheelBased", .type = 0, .order = 0, .mode = 0, .start = 0, .length = 16, .factor = 0.01, .offset = 0, .min = 0, .max = 2500, .unit = "km/h"}};

			show_table1(stable, 5, 64);
			igSameLine(0, 10);
			show_table2(stable, 5);
			igEndTabItem();
		}
		if (igBeginTabItem("Tab2", NULL, 0)) {
			igEndTabItem();
		}

		igEndTabBar();
	}
	igEnd();
}