#include "gui_interfaces.h"

#include <egquantities.h>
#include <egcan.h>
#include <egifaces.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>

typedef struct {
	char const *name;
	EgIfacesDetails *iface;
	ecs_entity_t e;
} gui_interface_table_t;

void gui_interfaces_progress(ecs_world_t *world, ecs_query_t *q)
{
	assert(world);
	assert(q);
	// int n = ecs_query_entity_count(q);
	gui_interface_table_t gui[128] = {0};

	int n = 0;
	ecs_iter_t it = ecs_query_iter(world, q);
	while (ecs_query_next(&it)) {
		EgIfacesDetails *iface = ecs_field(&it, EgIfacesDetails, 1);

		for (int i = 0; i < it.count; ++i, ++iface) {
			ecs_entity_t e = it.entities[i];
			int list_index = iface->index;
			if (list_index >= 128) {
				continue;
			}
			char const *name = ecs_get_name(world, e);
			// printf("e: %s, from : %s\n", name, name1 ? name1 : "?");
			gui[list_index].e = e;
			gui[list_index].name = name;
			gui[list_index].iface = iface;
			n = ECS_MAX(list_index, n);
		}
	}

	static ImGuiTableFlags flags2 = ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;

	if (igBeginTable("Signals table", 16, flags2, (ImVec2){0, 0}, 0)) {
		igTableSetupColumn("name", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("index", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("can_bitrate", ImGuiTableColumnFlags_WidthFixed, 60, 0);
		igTableSetupColumn("can_clock", ImGuiTableColumnFlags_WidthFixed, 60, 0);
		igTableSetupColumn("mtu", ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("minmtu", ImGuiTableColumnFlags_WidthFixed, 80, 0);
		igTableSetupColumn("maxmtu", ImGuiTableColumnFlags_WidthFixed, 80, 0);
		igTableSetupColumn("tso_max_size", ImGuiTableColumnFlags_WidthFixed, 100, 0);
		igTableSetupColumn("numtxqueues", ImGuiTableColumnFlags_WidthFixed, 100, 0);
		igTableSetupColumn("numrxqueues", ImGuiTableColumnFlags_WidthFixed, 100, 0);

		igTableSetupColumn("stats64_rx_bytes", ImGuiTableColumnFlags_WidthFixed, 100, 0);
		igTableSetupColumn("stats64_rx_packets", ImGuiTableColumnFlags_WidthFixed, 100, 0);
		igTableSetupColumn("stats64_rx_errors", ImGuiTableColumnFlags_WidthFixed, 100, 0);
		igTableSetupColumn("stats64_tx_bytes", ImGuiTableColumnFlags_WidthFixed, 100, 0);
		igTableSetupColumn("stats64_tx_packets", ImGuiTableColumnFlags_WidthFixed, 100, 0);
		igTableSetupColumn("stats64_tx_errors", ImGuiTableColumnFlags_WidthFixed, 100, 0);

		igTableHeadersRow();

		/*
		ecs_os_mutex_lock(stuff->lock);
		memcpy(book->tx, book->rx, sizeof(eg_can_book8_t) * book->cap);
		ecs_os_mutex_unlock(stuff->lock);
		*/

		for (int i = 0; i <= n; ++i) {
			// ecs_entity_t e = gui[i].e;
			char const *name = gui[i].name;
			EgIfacesDetails *iface = gui[i].iface;
			if (name) {
				igTableNextColumn();
				igText("%s", name);
				igTableNextColumn();
				igText("%i", iface->index);
				igTableNextColumn();
				igText("%i", iface->can_bitrate);
				igTableNextColumn();
				igText("%i", iface->can_clock);
				igTableNextColumn();
				igText("%i", iface->mtu);
				igTableNextColumn();
				igText("%i", iface->minmtu);
				igTableNextColumn();
				igText("%i", iface->maxmtu);
				igTableNextColumn();
				igText("%i", iface->tso_max_size);
				igTableNextColumn();
				igText("%i", iface->numtxqueues);
				igTableNextColumn();
				igText("%i", iface->numrxqueues);
				igTableNextColumn();
				igText("%i", iface->stats64_rx_bytes);
				igTableNextColumn();
				igText("%i", iface->stats64_rx_packets);
				igTableNextColumn();
				igText("%i", iface->stats64_rx_errors);
				igTableNextColumn();
				igText("%i", iface->stats64_tx_bytes);
				igTableNextColumn();
				igText("%i", iface->stats64_tx_packets);
				igTableNextColumn();
				igText("%i", iface->stats64_tx_errors);
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

ecs_query_t *gui_interfaces_query(ecs_world_t *world)
{
	// clang-format off
	ecs_query_t *q = ecs_query(world, {.filter.terms = {
		{.id = ecs_id(EgIfacesDetails)},
	}});
	// clang-format on
	return q;
}