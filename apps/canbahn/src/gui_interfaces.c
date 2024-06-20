#include "gui_interfaces.h"

#include <egquantities.h>
#include <egcan.h>
#include <egifaces.h>
#include <egcolors/eg_color.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>

#include "flecs_imgui.h"




typedef struct {
	char const *name;
	EgIfacesDetails *iface;
	ecs_entity_t e;
	bool check;
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

	if (igBeginTable("Signals table", 18, flags2, (ImVec2){0, 0}, 0)) {


		igTableSetupColumn("select", ImGuiTableColumnFlags_AngledHeader | ImGuiTableColumnFlags_WidthFixed, 30, 0);
		igTableSetupColumn("frame", ImGuiTableColumnFlags_AngledHeader | ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("index", ImGuiTableColumnFlags_AngledHeader | ImGuiTableColumnFlags_WidthFixed, 20, 0);
		igTableSetupColumn("link_type", ImGuiTableColumnFlags_AngledHeader | ImGuiTableColumnFlags_WidthFixed, 60, 0);
		igTableSetupColumn("can_bitrate", ImGuiTableColumnFlags_AngledHeader | ImGuiTableColumnFlags_WidthFixed, 70, 0);
		igTableSetupColumn("can_clock", ImGuiTableColumnFlags_AngledHeader | ImGuiTableColumnFlags_WidthFixed, 70, 0);
		igTableSetupColumn("mtu", ImGuiTableColumnFlags_AngledHeader | ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("min_mtu", ImGuiTableColumnFlags_AngledHeader | ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("max_mtu", ImGuiTableColumnFlags_AngledHeader | ImGuiTableColumnFlags_WidthFixed, 50, 0);
		igTableSetupColumn("tso_max_size", ImGuiTableColumnFlags_AngledHeader | ImGuiTableColumnFlags_WidthFixed, 100, 0);
		igTableSetupColumn("num_tx_queues", ImGuiTableColumnFlags_AngledHeader | ImGuiTableColumnFlags_WidthFixed, 20, 0);
		igTableSetupColumn("num_rx_queues", ImGuiTableColumnFlags_AngledHeader | ImGuiTableColumnFlags_WidthFixed, 20, 0);
		igTableSetupColumn("stats64_rx_bytes", ImGuiTableColumnFlags_AngledHeader | ImGuiTableColumnFlags_WidthFixed, 100, 0);
		igTableSetupColumn("stats64_rx_packets", ImGuiTableColumnFlags_AngledHeader | ImGuiTableColumnFlags_WidthFixed, 100, 0);
		igTableSetupColumn("stats64_rx_errors", ImGuiTableColumnFlags_AngledHeader | ImGuiTableColumnFlags_WidthFixed, 100, 0);
		igTableSetupColumn("stats64_tx_bytes", ImGuiTableColumnFlags_AngledHeader | ImGuiTableColumnFlags_WidthFixed, 100, 0);
		igTableSetupColumn("stats64_tx_packets", ImGuiTableColumnFlags_AngledHeader | ImGuiTableColumnFlags_WidthFixed, 100, 0);
		igTableSetupColumn("stats64_tx_errors", ImGuiTableColumnFlags_AngledHeader | ImGuiTableColumnFlags_WidthFixed, 100, 0);

		igTableAngledHeadersRow();
		igTableHeadersRow();

		for (int i = 0; i <= n; ++i) {
			char const *name = gui[i].name;
			EgIfacesDetails *iface = gui[i].iface;
			igPushID_Ptr(name);
			if (name) {
				igTableNextColumn();
				{
					char buf[128];
					snprintf(buf, sizeof(buf), "ifselected.%s", name);
					ecs_entity_t ee = ecs_lookup(world, buf);
					bool checked = !!ee;
					bool pressed = igCheckbox("", &checked);
					if (pressed) {
						printf("checked:%i:%s\n", checked, name);
					}
					if (checked) {
						ee = ecs_new_entity(world, buf);
					} else if(ee) {
						ecs_delete(world, ee);
					}
				}

				igTableNextColumn();
				igText("%s", name);
				igTableNextColumn();
				igText("%i", iface->index);
				igTableNextColumn();

				igPushStyleColor_U32_HSV_strhash(iface->link_type);
				igText("%s", iface->link_type);
				igPopStyleColor(1);

				igTableNextColumn();
				igText("%i", iface->can_bitrate);
				igTableNextColumn();
				igText("%i", iface->can_clock);
				igTableNextColumn();
				igText("%i", iface->mtu);
				igTableNextColumn();
				igText("%i", iface->min_mtu);
				igTableNextColumn();
				igText("%i", iface->max_mtu);
				igTableNextColumn();
				igText("%i", iface->tso_max_size);
				igTableNextColumn();
				igText("%i", iface->num_tx_queues);
				igTableNextColumn();
				igText("%i", iface->num_rx_queues);
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
				igTableNextColumn();
				igText("");
				igTableNextColumn();
				igText("");
			}
			igPopID();
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