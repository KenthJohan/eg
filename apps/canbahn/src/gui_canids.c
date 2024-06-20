#include "gui_canids.h"

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


int compare_canid(ecs_entity_t e1, const EgCanId *p1, ecs_entity_t e2, const EgCanId *p2)
{
	(void)e1;
	(void)e2;
	return (p1->id) < (p2->id);
}

void gui_canids_progress(ecs_world_t *world, ecs_query_t *q)
{
	assert(world);
	assert(q);
	// int n = ecs_query_entity_count(q);

	static ImGuiTableFlags flags2 = ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
	if (igBeginTable("Signals table", 9, flags2, (ImVec2){0, 0}, 0) == false) {
		return;
	}

	igTableSetupColumn("bus", ImGuiTableColumnFlags_WidthFixed, 50, 0);
	igTableSetupColumn("parent", ImGuiTableColumnFlags_WidthFixed, 100, 0);
	igTableSetupColumn("frame", ImGuiTableColumnFlags_WidthFixed, 250, 0);
	igTableSetupColumn("id10", ImGuiTableColumnFlags_WidthFixed, 50, 0);
	igTableSetupColumn("id16", ImGuiTableColumnFlags_WidthFixed, 50, 0);
	igTableSetupColumn("n", ImGuiTableColumnFlags_WidthFixed, 50, 0);
	igTableSetupColumn("f[Hz]", ImGuiTableColumnFlags_WidthFixed, 50, 0);
	igTableSetupColumn("T[ms]", ImGuiTableColumnFlags_WidthFixed, 50, 0);
	igTableSetupColumn("rx", ImGuiTableColumnFlags_WidthFixed, 300, 0);
	igTableHeadersRow();

	ecs_iter_t it = ecs_query_iter(world, q);
	while (ecs_query_next(&it)) {
		EgCanBus *bus = ecs_field(&it, EgCanBus, 1);   // shared
		EgCanId *channel = ecs_field(&it, EgCanId, 2); // self
		for (int i = 0; i < it.count; ++i, ++channel) {
			ecs_entity_t e = it.entities[i];
			ecs_entity_t ep = ecs_get_parent(world, e);
			char const * idname = ecs_get_name(world, e);
			char const * parentname = ecs_get_name(world, ep);
			igTableNextColumn();
			igText("%i", bus->socket);

			igTableNextColumn();
			igPushStyleColor_U32_HSV_strhash(parentname);
			igText("%s", parentname);
			igPopStyleColor(1);

			igTableNextColumn();
			igPushStyleColor_U32_HSV_strhash(idname);
			igText("%s", idname);
			igPopStyleColor(1);

			igTableNextColumn();
			igText("%3i", channel->id);
			igTableNextColumn();
			igText("%03X", channel->id);
			igTableNextColumn();
			igText("%u", channel->n);
			{
				uint32_t f = (uint32_t)(1.0 / (double)(channel->elapsed));
				uint32_t ms = (uint32_t)(channel->elapsed * 1000.0);
				igPushStyleColor_U32_HSV_hash32(f);
				igTableNextColumn();
				igText("%u", f);
				igTableNextColumn();
				igText("%u", ms);
				igPopStyleColor(1);
			}
			igTableNextColumn();
			eg_can_book_t * book = bus->ptr;
			eg_can_book_packet8_t * rx = NULL;
			if (channel->id < book->cap) {
				rx = book->rx + channel->id;
				uint8_t * p = rx->payload;
				igText("%02X %02X %02X %02X %02X %02X %02X %02X", p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
			}
		}
	}

	igEndTable();
}

ecs_query_t *gui_canids_query(ecs_world_t *world)
{
	// clang-format off
	ecs_query_t * q = ecs_query(world, {
		.filter.terms = {
		{.id = ecs_id(EgCanBus), .src.flags = EcsUp, .src.trav = EcsChildOf},
		{.id = ecs_id(EgCanId)},
		},
		//.order_by = (ecs_order_by_action_t)compare_canid,
		//.order_by_component = ecs_id(EgCanId) 
		}
	);
	// clang-format on
	return q;
}