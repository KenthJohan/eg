#include "gui_exporter.h"

#include <egquantities.h>
#include <egcan.h>
#include <egifaces.h>
#include <egcolors/eg_color.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>

#include "flecs_imgui.h"

void gui_exporter_progress(ecs_world_t *world, ecs_query_t *q, gui_exporter_t *exporter, bool *p_open)
{
	assert(world);
	assert(q);
	igSetNextWindowSize((ImVec2){600, 400}, 0);
	igBegin("Exporter", p_open, 0);
	igInputText("prefix", exporter->prefix, 128, 0, 0, 0);
	igInputText("destination C", exporter->export_destination_c, 128, 0, 0, 0);
	igInputText("destination Python", exporter->export_destination_python, 128, 0, 0, 0);
	igSeparator();

	if (igButton("Export C", (ImVec2){0, 0})) {
		FILE * f = fopen(exporter->export_destination_c, "w");
		fprintf(f, "%s", "#pragma once\n");
		fprintf(f, "%s", "// Generated CAN-ids\n");
		if (f) {
			ecs_iter_t it = ecs_query_iter(world, q);
			while (ecs_query_next(&it)) {
				EgCanId *channel = ecs_field(&it, EgCanId, 1); // self
				EcsIdentifier *ident = ecs_field(&it, EcsIdentifier, 2); // self
				EcsIdentifier *ident2 = ecs_field(&it, EcsIdentifier, 3); // shared
				for(int i = 0; i < it.count; ++i, ++channel, ++ident) {
					//printf("ID %i, %s_%s\n", channel->id, ident2->value, ident->value);
					fprintf(f, "%s", "#define ");
					char buf2[256];
					snprintf(buf2, 256, "%s_%s_%s", exporter->prefix, ident2->value, ident->value);
					fprintf(f, "%-40s 0x%03X\n", buf2, channel->id);
				}
			}
			fclose(f);
		}
	}
	
	if (igButton("Export Python", (ImVec2){0, 0})) {
		FILE * f = fopen(exporter->export_destination_python, "w");
		fprintf(f, "%s", "# Generated CAN-ids\n");
		if (f) {
			ecs_iter_t it = ecs_query_iter(world, q);
			while (ecs_query_next(&it)) {
				EgCanId *channel = ecs_field(&it, EgCanId, 1); // self
				EcsIdentifier *ident = ecs_field(&it, EcsIdentifier, 2); // self
				EcsIdentifier *ident2 = ecs_field(&it, EcsIdentifier, 3); // shared
				for(int i = 0; i < it.count; ++i, ++channel, ++ident) {
					//printf("ID %i, %s_%s\n", channel->id, ident2->value, ident->value);
					fprintf(f, "%s", "");
					char buf2[256];
					snprintf(buf2, 256, "%s_%s_%s", exporter->prefix, ident2->value, ident->value);
					fprintf(f, "%-40s = 0x%03X\n", buf2, channel->id);
				}
			}
			fclose(f);
		}
	}
	igEnd();
}

ecs_query_t *gui_exporter_query(ecs_world_t *world)
{
	// clang-format off
	ecs_query_t *q = ecs_query(world, {.filter.terms = {
		{.id = ecs_id(EgCanId)},
		{.id = ecs_pair(ecs_id(EcsIdentifier), EcsName)},
		{.id = ecs_pair(ecs_id(EcsIdentifier), EcsName), .src.flags = EcsUp, .src.trav = EcsChildOf},
	}});
	// clang-format on
	return q;
}