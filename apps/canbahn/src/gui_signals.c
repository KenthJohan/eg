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

#define COLOR_RGBA(r, g, b, a) ((r) << 0 | (g) << 8 | (b) << 16 | (a) << 24)

typedef struct {
	char const *signame;
	char const *idname;
	EgCanBusDescription *desc;
	EgCanBus *bus;
	EgCanId *channel;
	EgCanSignal *signal;
	GuiCanTableRow *row;
	EgQuantitiesRangedGeneric *value;
	EgQuantitiesIsq *q;
	ecs_entity_t e;
} gui_can_table_t;

static void gui_tx(ecs_world_t *world, eg_can_book_t *book, EgCanId *channel, EgCanSignal *signal, GuiCanTableRow *row, EgQuantitiesRangedGeneric *value)
{
	bool modifed = false;
	if (value) {
		if (signal->rxtx & 0x02) {
			if (row->kind) {
				EcsEnum const *enum_type = ecs_get(world, row->kind, EcsEnum);
				if (enum_type) {
					int v = (int)value->tx.val_u64;
					modifed = igCombo_flecs(world, enum_type, &v);
					if (modifed) {
						value->tx.val_u64 = (int32_t)v;
					}
				}
			} else {
				modifed = igSlider_flecs("##s1", value);
			}
		}

		if (signal->rxtx & 0x04) {
			modifed = igButton("RTR", (ImVec2){0, 0});
		}
	} else {
		modifed = igButton("Send", (ImVec2){0, 0});
	}

	if (modifed) {
		EgCan_book_prepare_send(book, signal, channel, value);
	}
}

static void gui_rx(ecs_world_t *world, eg_can_book_t *book, EgCanSignal *signal, GuiCanTableRow *row, EgQuantitiesRangedGeneric *value)
{
	if (value) {
		if (signal->rxtx & 0x01) {
			igText_flecs_enum(world, row->kind, &value->rx, value->kind);
		}
	}
}

#define MAX_ROWS 256

void gui_signals_progress(ecs_world_t *world, ecs_query_t *q)
{
	assert(world);
	assert(q);
	// int n = ecs_query_entity_count(q);

	static ImGuiTableFlags flags2 = ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
	if (igBeginTable("Signals table", 16, flags2, (ImVec2){0, 0}, 0)) {
		gui_can_table_t gui[MAX_ROWS] = {0};
		int n = 0;
		ecs_iter_t it = ecs_query_iter(world, q);
		while (ecs_query_next(&it)) {
			EgCanBus *bus = ecs_field(&it, EgCanBus, 1);                                     // shared
			EgCanBusDescription *desc = ecs_field(&it, EgCanBusDescription, 2);              // shared
			EgCanId *channel = ecs_field(&it, EgCanId, 3);                                   // shared
			EgCanSignal *signal = ecs_field(&it, EgCanSignal, 4);                            // self
			GuiCanTableRow *row = ecs_field(&it, GuiCanTableRow, 5);                         // self
			EgQuantitiesIsq *quant = ecs_field(&it, EgQuantitiesIsq, 6);                     // self, optional
			EgQuantitiesRangedGeneric *value = ecs_field(&it, EgQuantitiesRangedGeneric, 7); // self, optional

			for (int i = 0; i < it.count; ++i, ++signal, ++row, quant += (quant != NULL), value += (value != NULL)) {
				ecs_entity_t e = it.entities[i];
				if (row->index >= MAX_ROWS) {
					continue;
				}
				// char const *name = ecs_get_path_w_sep(world, row->parent, e, ".", NULL);
				// printf("e: %s, from : %s\n", name, name1 ? name1 : "?");
				// char const *name = ecs_get_path_w_sep(world, row->parent, e, ".", NULL);
				ecs_entity_t ident = ecs_field_src(&it, 3);
				gui[row->index].e = e;
				gui[row->index].signame = ecs_get_name(world, e);
				gui[row->index].idname = ecs_get_name(world, ident);
				gui[row->index].channel = channel;
				gui[row->index].signal = signal;
				gui[row->index].value = value;
				gui[row->index].q = quant;
				gui[row->index].desc = desc;
				gui[row->index].bus = bus;
				gui[row->index].row = row;
				n = ECS_MAX(row->index + 1, n);
			}
		}
		igTableSetupColumn("bus", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultHide, 50, 0);
		igTableSetupColumn("sock", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultHide, 50, 0);
		igTableSetupColumn("idname", ImGuiTableColumnFlags_WidthFixed, 150, 0);
		igTableSetupColumn("id10", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("id16", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("idn", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("signame", ImGuiTableColumnFlags_WidthFixed, 150, 0);
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

		for (int i = 0; i < n; ++i) {
			char const *signame = gui[i].signame;
			char const *idname = gui[i].idname;
			EgQuantitiesIsq *quant = gui[i].q;
			EgCanId *channel = gui[i].channel;
			EgCanSignal *signal = gui[i].signal;
			GuiCanTableRow *row = gui[i].row;
			EgQuantitiesRangedGeneric *value = gui[i].value;
			EgCanBus *bus = gui[i].bus;
			EgCanBusDescription *desc = gui[i].desc;
			ecs_entity_t e = gui[i].e;

			if (signame == NULL) {
				igTableNextColumn();
				igTableNextColumn();
				igTableNextColumn();
				igTableNextColumn();
				igTableNextColumn();
				igTableNextColumn();
				igTableNextColumn();
				igTableNextColumn();
				igTableNextColumn();
				igTableNextColumn();
				igTableNextColumn();
				igTableNextColumn();
				igTableNextColumn();
				igTableNextColumn();
				igTableNextColumn();
				igTableNextColumn();
				continue;
			}

			eg_can_book_t *book = bus->ptr;

			igPushID_Ptr(signal);
			// igTableSetBgColor(ImGuiTableBgTarget_RowBg0, COLOR_RGBA(0xFF, 0xFF, 0xFF, 0xFF), -1);

			igTableNextColumn();
			igText(desc->interface);

			igTableNextColumn();
			igText("%i", bus->socket);

			igPushStyleColor_U32_HSV_hash32(channel->id);
			{
				igTableNextColumn();
				igText("%s", idname);
				igTableNextColumn();
				igText("%i", channel->id);
				igTableNextColumn();
				igText("%03X", channel->id);
				igTableNextColumn();
				igText("%i", channel->n);
			}
			igPopStyleColor(1);


			igTableNextColumn();
			igText("%s", signame);

			igTableNextColumn();
			igText("%i", signal->byte_offset);

			igTableNextColumn();
			if (value) {
				igPushStyleColor_U32_HSV_strhash(flecs_get_type(value->kind));
				{
					igText("%s", flecs_get_type(value->kind));
				}
				igPopStyleColor(1);
			}

			igTableNextColumn();
			if (value) {
				igText_flecs(&value->min, value->kind);
			}

			igTableNextColumn();
			if (value) {
				igText_flecs(&value->max, value->kind);
			}

			igTableNextColumn();
			gui_tx(world, book, channel, signal, row, value);

			igTableNextColumn();
			gui_rx(world, book, signal, row, value);

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
			{.id = ecs_id(EgCanId), .src.flags = EcsUp, .src.trav = EcsChildOf},
			{.id = ecs_id(EgCanSignal), .src.flags = EcsSelf}, // EcsSelf is temporary fix to only query from "app.signals".
			{.id = ecs_id(GuiCanTableRow), .src.flags = EcsSelf}, // EcsSelf is temporary fix to only query from "app.signals".
			{.id = ecs_id(EgQuantitiesIsq), .oper = EcsOptional},
			{.id = ecs_id(EgQuantitiesRangedGeneric), .oper = EcsOptional, .src.flags = EcsSelf}, // EcsSelf is temporary fix to only query from "app.signals".
			//{.id = ecs_id(EgCanSignal)},
			// TODO: Only query entities from "app.signals"
			//{.id = EcsModule, .src.id = e_app_signals},
		}
		}
	);
	// clang-format on
	return q;
}