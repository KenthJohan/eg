#include "gui_signals.h"

#include <egquantities.h>
#include <egcan.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>


typedef struct {
	char const *name;
	EgCanBusDescription *desc;
	EgCanBus * bus;
	EgCanSignal *signal;
	EgQuantitiesIsq *q;
	EgCanBusBook * book;
	ecs_entity_t e;
} gui_can_table_t;



void gui_signals_progress(ecs_world_t *world, ecs_query_t *q)
{
	assert(world);
	assert(q);
	// int n = ecs_query_entity_count(q);
	gui_can_table_t gui[128] = {0};

	int n = 0;
	ecs_iter_t it = ecs_query_iter(world, q);
	while (ecs_query_next(&it)) {
		EgCanBus *bus = ecs_field(&it, EgCanBus, 1);
		EgCanBusDescription *desc = ecs_field(&it, EgCanBusDescription, 2);
		EgCanBusBook *book = ecs_field(&it, EgCanBusBook, 3);
		EgCanSignal *signal = ecs_field(&it, EgCanSignal, 4);
		EgQuantitiesIsq *quant = ecs_field(&it, EgQuantitiesIsq, 5);

		for (int i = 0; i < it.count; ++i, ++signal, ++quant) {
			ecs_entity_t e = it.entities[i];
			int list_index = signal->gui_index;
			if (list_index >= 128) {
				continue;
			}
			char const * name = ecs_get_name(world, e);
			//printf("e: %s, from : %s\n", name, name1 ? name1 : "?");
			gui[list_index].e = e;
			gui[list_index].name = name;
			gui[list_index].signal = signal;
			gui[list_index].q = quant;
			gui[list_index].desc = desc;
			gui[list_index].bus = bus;
			gui[list_index].book = book;
			n = ECS_MAX(list_index, n);
		}
	}


	
	static ImGuiTableFlags flags2 = ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;

	if (igBeginTable("Signals table", 11, flags2, (ImVec2){0, 0}, 0)) {
		igTableSetupColumn("name", ImGuiTableColumnFlags_WidthFixed, 100, 0);
		igTableSetupColumn("bus", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("sock", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("canid", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("o", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("min", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("max", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("tx", 0, 0, 0);
		igTableSetupColumn("rx", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("q", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("u", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableHeadersRow();

		/*
		ecs_os_mutex_lock(stuff->lock);
		memcpy(book->tx, book->rx, sizeof(eg_can_book8_t) * book->cap);
		ecs_os_mutex_unlock(stuff->lock);
		*/

		for (int i = 0; i <= n; ++i) {
			//ecs_entity_t e = gui[i].e;
			char const * name = gui[i].name;
			EgQuantitiesIsq *quant = gui[i].q;
			EgCanSignal *signal = gui[i].signal;
			EgCanBus *bus = gui[i].bus;
			EgCanBusDescription *desc = gui[i].desc;
			EgCanBusBook *book = gui[i].book;
			if(name) {
				igPushID_Ptr(signal);
				igTableNextColumn();
				igText("%s", name);
				igTableNextColumn();
				igText(desc->interface);
				igTableNextColumn();
				igText("%i", bus->socket);
				igTableNextColumn();
				igText("%i", signal->canid);
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
					if (igSliderScalar("", ImGuiDataType_S32, &signal->tx, &signal->min, &signal->max, "%d", 0)) {
						EgCanBusBook_prepare_send(book, signal);
					};
					igPopItemWidth();
				} else {
					igText("");
				}

				igTableNextColumn();
				igText("%i", signal->rx);
				igTableNextColumn();
				igText(quant ? quant->symbol : "");
				igTableNextColumn();
				igText("");
				igPopID();
			} else {
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
			}
		}


		igEndTable();
	}
}



ecs_query_t * gui_signals_query(ecs_world_t *world)
{
	ecs_query_t * q = ecs_query(world, {
		.filter.terms = {
			{.id = ecs_id(EgCanBus), .src.flags = EcsUp, .src.trav = EcsIsA},
			{.id = ecs_id(EgCanBusDescription), .src.flags = EcsUp, .src.trav = EcsIsA},
			{.id = ecs_id(EgCanBusBook), .src.flags = EcsUp, .src.trav = EcsIsA},
			{.id = ecs_id(EgCanSignal), .src.flags = EcsSelf}, // EcsSelf is temporary fix to only query from "app.signals".
			//{.id = ecs_id(EgCanSignal)},
			{.id = ecs_id(EgQuantitiesIsq), .oper = EcsOptional},
			// TODO: Only query entities from "app.signals"
			//{.id = EcsModule, .src.id = e_app_signals},
		}
		}
	);
	return q;
}