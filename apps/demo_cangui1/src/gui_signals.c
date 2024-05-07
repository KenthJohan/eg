#include "gui_signals.h"

#include <egquantities.h>
#include <egcan.h>

#include <egcolors/eg_color.h>

#define COLOR_RGBA(r,g,b,a) ((r) << 0 | (g) << 8 | (b) << 16 | (a) << 24)

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>

#include <stdlib.h>
#include "GuiCan.h"



typedef struct {
	char const *name;
	EgCanBusDescription *desc;
	EgCanBus *bus;
	EgCanSignal *signal;
	EgQuantitiesIsq *q;
	ecs_entity_t e;
} gui_can_table_t;

void gui_signals_progress(ecs_world_t *world, ecs_query_t *q)
{
	assert(world);
	assert(q);
	// int n = ecs_query_entity_count(q);

	static ImGuiTableFlags flags2 = ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
	if (igBeginTable("Signals table", 13, flags2, (ImVec2){0, 0}, 0)) {
		gui_can_table_t gui[128] = {0};
		int n = 0;
		ecs_iter_t it = ecs_query_iter(world, q);
		while (ecs_query_next(&it)) {
			EgCanBus *bus = ecs_field(&it, EgCanBus, 1); // shared
			EgCanBusDescription *desc = ecs_field(&it, EgCanBusDescription, 2); // shared
			EgCanSignal *signal = ecs_field(&it, EgCanSignal, 3);  // self
			EgQuantitiesIsq *quant = ecs_field(&it, EgQuantitiesIsq, 4); // self

			for (int i = 0; i < it.count; ++i, ++signal, ++quant) {
				ecs_entity_t e = it.entities[i];
				int list_index = signal->gui_index;
				if (list_index >= 128) {
					continue;
				}
				char const *name = ecs_get_name(world, e);
				// printf("e: %s, from : %s\n", name, name1 ? name1 : "?");
				gui[list_index].e = e;
				gui[list_index].name = name;
				gui[list_index].signal = signal;
				gui[list_index].q = quant;
				gui[list_index].desc = desc;
				gui[list_index].bus = bus;
				n = ECS_MAX(list_index, n);
			}
		}


	
		igTableSetupColumn("name", ImGuiTableColumnFlags_WidthFixed, 100, 0);
		igTableSetupColumn("bus", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("sock", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("canid", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("idn", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("o", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("min", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("max", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("tx", ImGuiTableColumnFlags_WidthFixed, 200, 0);
		igTableSetupColumn("rx", ImGuiTableColumnFlags_WidthFixed, 200, 0);
		igTableSetupColumn("q", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("u", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("plot", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableHeadersRow();

		/*
		ecs_os_mutex_lock(stuff->lock);
		memcpy(book->tx, book->rx, sizeof(eg_can_book8_t) * book->cap);
		ecs_os_mutex_unlock(stuff->lock);
		*/

		for (int i = 0; i < n; ++i) {
			// ecs_entity_t e = gui[i].e;
			char const *name = gui[i].name;
			EgQuantitiesIsq *quant = gui[i].q;
			EgCanSignal *signal = gui[i].signal;
			EgCanBus *bus = gui[i].bus;
			EgCanBusDescription *desc = gui[i].desc;

			if (name == NULL) {
				igTableNextColumn();
				igText("");
				igTableNextColumn();
				igText("");
				igTableNextColumn();
				igText("");
				igTableNextColumn();
				igText("");
				igTableNextColumn();
				igText("");
				igTableNextColumn();
				igText("");
				igTableNextColumn();
				igText("");
				igTableNextColumn();
				igText("");
				igTableNextColumn();
				igText("");
				igTableNextColumn();
				igText("");
				igTableNextColumn();
				igText("");
				igTableNextColumn();
				igText("");
				igTableNextColumn();
				igText("");
				continue;
			}
			
			eg_can_book_t *book = bus->ptr;
			igPushID_Ptr(signal);
			igTableNextColumn();
			igText("%s", name);
			igTableNextColumn();
			igText(desc->interface);
			igTableNextColumn();
			igText("%i", bus->socket);
			igTableNextColumn();
			{
				uint8_t h = (signal->canid<<6) + (signal->canid << 16) - signal->canid;
				uint8_t r;
				uint8_t g;
				uint8_t b;
				eg_color_hsv_to_rgb(h, 255, 255, &r, &g, &b);
				igPushStyleColor_U32(ImGuiCol_Text, COLOR_RGBA(r,g,b,255));
				igText("%i", signal->canid);
				igPopStyleColor(1);
			}
			igTableNextColumn();
			igText("%i", signal->idn);
			igTableNextColumn();
			igText("%i", signal->byte_offset);

			igTableNextColumn();
			igPushItemWidth(-1);
			igInputInt("#1", &signal->min, 0, 0, 0);
			igPopItemWidth();
			igTableNextColumn();
			igPushItemWidth(-1);
			igInputInt("#2", &signal->max, 0, 0, 0);
			igPopItemWidth();

			igTableNextColumn();
			if (signal->min != signal->max) {
				igPushItemWidth(-1);
				if (igSliderScalar("##s1", ImGuiDataType_S32, &signal->tx, &signal->min, &signal->max, "%d", 0)) {
					EgCan_book_prepare_send(book, signal);
				};
				igPopItemWidth();
			} else {
				igText("");
			}

			igTableNextColumn();

			igText("%i", signal->rx);
			/*
			if (signal->min != signal->max) {
				igBeginDisabled(true);
				igPushItemWidth(-1);
				igSliderScalar("##s2", ImGuiDataType_S32, &signal->rx, &signal->min, &signal->max, "%d", 0);
				igPopItemWidth();
				igEndDisabled();
			} else {
				igText("");
			}
			*/

			/*
			igTableNextColumn();
			igBeginDisabled(true);
			igText("%i", signal->rx);
			igEndDisabled();
			*/

			igTableNextColumn();
			igText(quant ? quant->symbol : "");
			igTableNextColumn();
			igText("");
			
			igTableNextColumn();
			{
				bool checked = ecs_has(world, gui[i].e, GuiCanPlot);
				bool pressed = igCheckbox("", &checked);
				if (pressed) {
					printf("checked:%i:%s\n", checked, name);
					if (checked) {
						ecs_add(world, gui[i].e, GuiCanPlot);
					} else {
						ecs_remove(world, gui[i].e, GuiCanPlot);
					}
				}
			}

			igPopID();
		}

		igEndTable();
	}
}

ecs_query_t *gui_signals_query(ecs_world_t *world)
{
	// clang-format off
	ecs_query_t * q = ecs_query(world, {
		.filter.terms = {
			{.id = ecs_id(EgCanBus), .src.flags = EcsUp, .src.trav = EcsChildOf},
			{.id = ecs_id(EgCanBusDescription), .src.flags = EcsUp, .src.trav = EcsChildOf},
			{.id = ecs_id(EgCanSignal), .src.flags = EcsSelf}, // EcsSelf is temporary fix to only query from "app.signals".
			//{.id = ecs_id(EgCanSignal)},
			{.id = ecs_id(EgQuantitiesIsq), .oper = EcsOptional},
			// TODO: Only query entities from "app.signals"
			//{.id = EcsModule, .src.id = e_app_signals},
		}
		}
	);
	// clang-format on
	return q;
}