#include "app_gui_cansig.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <egimgui.h>

#include "ig.h"

/*
https://www.csselectronics.com/pages/dbc-editor-can-bus-database
*/
void app_gui_cansig_table1(app_gui_cansig_state_t guisigs[], CanDbcSignal metas[], int cansig_count, int message_length)
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

	for (int i = 0; i < cansig_count; ++i) {
		guisigs[i].hover2 = false;
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
			int sigint = dbcsig_meta_bitpos_to_signal(metas, cansig_count, bitpos);

			char buf[128];
			snprintf(buf, 128, "%i", sigint);
			igSelectable_Bool(buf, false, 0, (ImVec2){0, 0});

			bitpos++;
			// Change background of Cells B1->C2
			// Demonstrate setting a cell background color with 'ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ...)'
			// (the CellBg color will be blended over the RowBg and ColumnBg colors)
			// We can also pass a column number as a third parameter to TableSetBgColor() and do this outside the column loop.
			// if (row >= 1 && row <= 2 && column >= 1 && column <= 2) {
			//	ImU32 cell_bg_color = igGetColorU32_Vec4((ImVec4){0.3f, 0.3f, 0.7f, 0.65f});
			//	igTableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color, -1);
			//}
			if (sigint >= 0) {
				guisigs[sigint].hover2 |= igIsItemHovered(0);
				guisigs[sigint].clicked ^= igIsItemClicked(0);

				if (guisigs[sigint].clicked) {
					igTableSetBgColor(ImGuiTableBgTarget_CellBg, hash32_to_color32(sigint, 255), -1);
				} else if (guisigs[sigint].hover1) {
					igTableSetBgColor(ImGuiTableBgTarget_CellBg, hash32_to_color32(sigint, 255), -1);
				} else if (guisigs[sigint].hover2) {
					igTableSetBgColor(ImGuiTableBgTarget_CellBg, hash32_to_color32(sigint, 255), -1);
				} else {
					igTableSetBgColor(ImGuiTableBgTarget_CellBg, hash32_to_color32(sigint, 100), -1);
				}

				if (guisigs[sigint].hover2) {
					// printf("sigint %i!\n", sigint);
				}
			}

			igPopID();
		}
		igPopID();
	}
	igEndTable();
}

void app_gui_cansig_table2(app_gui_cansig_state_t guisigs[], CanDbcSignal metas[], int cansig_count)
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

	float row_min_height = 24.0f;

	for (int i = 0; i < cansig_count; ++i) {
		CanDbcSignal *meta = metas + i;
		app_gui_cansig_state_t *guisig = guisigs + i;

		igTableNextRow(ImGuiTableRowFlags_None, row_min_height);
		igPushID_Int(i);

		igTableNextColumn();
		igTableSetBgColor(ImGuiTableBgTarget_CellBg, hash32_to_color32(i, 150), -1);
		generic_gui(&(generic_gui_t){
		.label = "##Select",
		.kind = GENERIC_GUI_KIND_TEXT_INT,
		.text_int.value = i});

		if (guisig->hover1) {
			igTableSetBgColor(ImGuiTableBgTarget_RowBg1, hash32_to_color32(i, 255), -1);
		}
		if (guisig->hover2) {
			igTableSetBgColor(ImGuiTableBgTarget_RowBg1, hash32_to_color32(i, 255), -1);
		}
		if (guisig->clicked) {
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
		guisig->hover1 = false;
	}

	int ri = igTableGetHoveredRow() - 1;
	if ((ri >= 0) && (ri < cansig_count)) {
		guisigs[ri].hover1 = true;
		guisigs[ri].clicked ^= igIsMouseReleased_ID(ImGuiMouseButton_Left, 0);
	}

	igEndTable();

	// ig_debug_draw();
}

















void app_gui_cansig_table3(app_gui_cansig_state_t guisigs[], CanDbcSignal metas[], int cansig_count)
{
	//igPushItemWidth(200);
	
	igBeginGroup();
	igPushItemWidth(40);
	for (int i = 0; i < cansig_count; ++i) {
		CanDbcSignal *meta = metas + i;
		app_gui_cansig_state_t *guisig = guisigs + i;
		generic_gui(&(generic_gui_t){
		.label = "##Select",
		.kind = GENERIC_GUI_KIND_TEXT_INT,
		.text_int.value = i});
	}
	igPopItemWidth();
	igEndGroup();

	igSameLine(0, 10);

	igBeginGroup();
	igPushItemWidth(40);
	for (int i = 0; i < cansig_count; ++i) {
		CanDbcSignal *meta = metas + i;
		app_gui_cansig_state_t *guisig = guisigs + i;
		generic_gui(&(generic_gui_t){
		.label = "##Name",
		.id = i,
		.kind = GENERIC_GUI_KIND_INPUT_TEXT,
		.input.data = meta->name,
		.input.data_size = 128,
		});
	}
	igPopItemWidth();
	igEndGroup();

	igSameLine(0, 10);

	igBeginGroup();
	igPushItemWidth(40);
	for (int i = 0; i < cansig_count; ++i) {
		CanDbcSignal *meta = metas + i;
		app_gui_cansig_state_t *guisig = guisigs + i;
		generic_gui(&(generic_gui_t){
		.label = "##Start",
		.id = i,
		.kind = GENERIC_GUI_KIND_INPUT_INT,
		.input.data = &meta->start,
		.input.data_size = sizeof(meta->start),
		});
	}
	igPopItemWidth();
	igEndGroup();

	igSameLine(0, 10);

	igBeginGroup();
	igPushItemWidth(40);
	for (int i = 0; i < cansig_count; ++i) {
		CanDbcSignal *meta = metas + i;
		app_gui_cansig_state_t *guisig = guisigs + i;
		generic_gui(&(generic_gui_t){
		.label = "##Length",
		.id = i,
		.kind = GENERIC_GUI_KIND_INPUT_INT,
		.input.data = &meta->length,
		.input.data_size = sizeof(meta->length),
		});
	}
	igPopItemWidth();
	igEndGroup();



	//igPopItemWidth();

	// ig_debug_draw();
}
