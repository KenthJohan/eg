#include "gui_signals.h"

#include <egquantities.h>
#include <egcan.h>

#include <egcolors/eg_color.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>

#include <stdlib.h>
#include <float.h>
#include <math.h>
#include "GuiCan.h"
#include "flecs_imgui.h"

typedef struct {
	char const *name;
	EgCanBusDescription *desc;
	EgCanBus *bus;
	EgCanSignal *signal;
	EgQuantitiesRangedGeneric *value;
	EgQuantitiesIsq *q;
	ecs_entity_t e;
} gui_can_table_t;

void gui_signals_progress(ecs_world_t *world, ecs_query_t *q)
{
	assert(world);
	assert(q);
	// int n = ecs_query_entity_count(q);

	static ImGuiTableFlags flags2 = ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
	if (igBeginTable("Signals table", 15, flags2, (ImVec2){0, 0}, 0)) {
		gui_can_table_t gui[128] = {0};
		int n = 0;
		ecs_iter_t it = ecs_query_iter(world, q);
		while (ecs_query_next(&it)) {
			EgCanBus *bus = ecs_field(&it, EgCanBus, 1);                                     // shared
			EgCanBusDescription *desc = ecs_field(&it, EgCanBusDescription, 2);              // shared
			EgCanSignal *signal = ecs_field(&it, EgCanSignal, 3);                            // self
			EgQuantitiesIsq *quant = ecs_field(&it, EgQuantitiesIsq, 4);                     // self, optional
			EgQuantitiesRangedGeneric *value = ecs_field(&it, EgQuantitiesRangedGeneric, 5); // self

			for (int i = 0; i < it.count; ++i, ++signal, ++quant, ++value) {
				ecs_entity_t e = it.entities[i];
				int list_index = signal->gui_index;
				if (list_index >= 128) {
					continue;
				}
				char const *name = ecs_get_path_w_sep(world, signal->gui_scope_name_parent, e, ".", NULL);
				// printf("e: %s, from : %s\n", name, name1 ? name1 : "?");
				gui[list_index].e = e;
				gui[list_index].name = name;
				gui[list_index].signal = signal;
				gui[list_index].value = value;
				gui[list_index].q = quant;
				gui[list_index].desc = desc;
				gui[list_index].bus = bus;
				n = ECS_MAX(list_index + 1, n);
			}
		}
		/*
		igTableSetColumnEnabled(0, false);
		igTableSetColumnEnabled(1, false);
		igTableSetColumnEnabled(2, false);
		igTableSetColumnEnabled(3, false);
		igTableSetColumnEnabled(4, true);
		igTableSetColumnEnabled(5, true);
		igTableSetColumnEnabled(6, false);
		igTableSetColumnEnabled(7, true);
		igTableSetColumnEnabled(8, false);
		igTableSetColumnEnabled(9, false);
		igTableSetColumnEnabled(10, true);
		igTableSetColumnEnabled(11, true);
		igTableSetColumnEnabled(12, false);
		igTableSetColumnEnabled(13, false);
		igTableSetColumnEnabled(14, true);
		*/
		igTableSetupColumn("name", ImGuiTableColumnFlags_WidthFixed, 200, 0);
		igTableSetupColumn("bus", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultHide, 50, 0);
		igTableSetupColumn("sock", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultHide, 50, 0);
		igTableSetupColumn("id10", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("id16", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("idn", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("o", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultHide, 50, 0);
		igTableSetupColumn("kind", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("min", ImGuiTableColumnFlags_WidthFixed, 200, 0);
		igTableSetupColumn("max", ImGuiTableColumnFlags_WidthFixed, 200, 0);
		igTableSetupColumn("tx", ImGuiTableColumnFlags_WidthFixed, 200, 0);
		igTableSetupColumn("rx", ImGuiTableColumnFlags_WidthFixed, 200, 0);
		igTableSetupColumn("q", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultHide, 50, 0);
		igTableSetupColumn("u", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultHide, 50, 0);
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
			EgQuantitiesRangedGeneric *value = gui[i].value;
			EgCanBus *bus = gui[i].bus;
			EgCanBusDescription *desc = gui[i].desc;
			ecs_entity_t e = gui[i].e;

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

			igPushStyleColor_U32_HSV_hash32(signal->canid);
			igTableNextColumn();
			igText("%i", signal->canid);
			igTableNextColumn();
			igText("%03X", signal->canid);
			igPopStyleColor(1);

			igTableNextColumn();
			igText("%i", signal->idn);
			igTableNextColumn();
			igText("%i", signal->byte_offset);
			igTableNextColumn();
			igPushStyleColor_U32_HSV_strhash(flecs_get_type(value->kind));
			igText("%s", flecs_get_type(value->kind));
			igPopStyleColor(1);

			igTableNextColumn();
			igText_flecs(&value->min, value->kind);

			igTableNextColumn();
			igText_flecs(&value->max, value->kind);

			igTableNextColumn();
			if (signal->rxtx & 0x02) {
				bool modifed = false;
				if (signal->component_rep && ecs_has(world, signal->component_rep, EcsEnum)) {
					int v = (int)value->tx.val_u64;
					modifed = igCombo_flecs(world, signal->component_rep, &v);
					if (modifed) {
						value->tx.val_u64 = (int32_t)v;
					}
				} else {
					modifed = igSlider_flecs("##s1", value);
				}

				if (modifed) {
					EgCan_book_prepare_send(book, signal, value);
				}
			}

			/*
			if (igSliderScalar("##s1", ImGuiDataType_Float, &value->tx.val_f32, &value->min.val_f32, &value->max.val_f32, "%f", 0)) {
			    EgCan_book_prepare_send(book, signal, value);
			};
			*/

			igTableNextColumn();
			if (signal->rxtx & 0x01) {
				igText_flecs_enum(world, signal->component_rep, &value->rx, value->kind);
			}
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
			igCheckbox_flecs(world, e, ecs_id(GuiCanPlot));

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
			{.id = ecs_id(EgQuantitiesIsq), .oper = EcsOptional},
			{.id = ecs_id(EgQuantitiesRangedGeneric), .src.flags = EcsSelf}, // EcsSelf is temporary fix to only query from "app.signals".
			//{.id = ecs_id(EgCanSignal)},
			// TODO: Only query entities from "app.signals"
			//{.id = EcsModule, .src.id = e_app_signals},
		}
		}
	);
	// clang-format on
	return q;
}