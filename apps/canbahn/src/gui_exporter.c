#include "gui_exporter.h"

#include <egquantities.h>
#include <egcan.h>
#include <egifaces.h>
#include <egstr.h>
#include <egcolors/eg_color.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>

#include "flecs_imgui.h"

static void export_c(ecs_world_t *world, ecs_query_t *q, char const *path, char const *prefix)
{
	FILE *f = fopen(path, "w");
	fprintf(f, "%s", "#pragma once\n");
	fprintf(f, "%s", "// Generated CAN-ids\n");
	if (f) {
		ecs_iter_t it = ecs_query_iter(world, q);
		while (ecs_query_next(&it)) {
			EgCanId *channel = ecs_field(&it, EgCanId, 1);            // self
			EcsIdentifier *ident = ecs_field(&it, EcsIdentifier, 2);  // self
			EcsIdentifier *ident2 = ecs_field(&it, EcsIdentifier, 3); // shared
			for (int i = 0; i < it.count; ++i, ++channel, ++ident) {
				// printf("ID %i, %s_%s\n", channel->id, ident2->value, ident->value);
				fprintf(f, "%s", "#define ");
				char buf2[256];
				snprintf(buf2, 256, "%s_%s_%s", prefix, ident2->value, ident->value);
				fprintf(f, "%-40s 0x%03X\n", buf2, channel->id);
			}
		}
		fclose(f);
	}
}

static void export_python(ecs_world_t *world, ecs_query_t *q, char const *path, char const *prefix)
{
	FILE *f = fopen(path, "w");
	fprintf(f, "%s", "#pragma once\n");
	fprintf(f, "%s", "// Generated CAN-ids\n");
	if (f) {
		ecs_iter_t it = ecs_query_iter(world, q);
		while (ecs_query_next(&it)) {
			EgCanId *channel = ecs_field(&it, EgCanId, 1);            // self
			EcsIdentifier *ident = ecs_field(&it, EcsIdentifier, 2);  // self
			EcsIdentifier *ident2 = ecs_field(&it, EcsIdentifier, 3); // shared
			for (int i = 0; i < it.count; ++i, ++channel, ++ident) {
				// printf("ID %i, %s_%s\n", channel->id, ident2->value, ident->value);
				fprintf(f, "%s", "#define ");
				char buf2[256];
				snprintf(buf2, 256, "%s_%s_%s", prefix, ident2->value, ident->value);
				fprintf(f, "%-40s 0x%03X\n", buf2, channel->id);
			}
		}
		fclose(f);
	}
}

static void text(char const * label, EgStrText *text)
{
	char buf[256];
	snprintf(buf, 256, "%s", text->value);
	ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue & 0;
	igPushItemWidth(-1);
	if (igInputText(label, buf, 128, flags, 0, 0)) {
		ecs_os_strset(&text->value, buf);
	}
	igPopItemWidth();
}

void gui_exporter_progress(ecs_world_t *world, ecs_query_t *q, ecs_query_t *q2)
{
	assert(world);
	assert(q);

	static ImGuiTableFlags flags2 = ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
	if (igBeginTable("Export table", 4, flags2, (ImVec2){0, 0}, 0) == false) {
		return;
	}
	igTableSetupColumn("name", ImGuiTableColumnFlags_WidthFixed, 100, 0);
	igTableSetupColumn("path", ImGuiTableColumnFlags_WidthFixed, 300, 0);
	igTableSetupColumn("prefix", ImGuiTableColumnFlags_WidthFixed, 200, 0);
	igTableSetupColumn("export", ImGuiTableColumnFlags_WidthFixed, 100, 0);
	igTableHeadersRow();

	ecs_iter_t it = ecs_query_iter(world, q2);
	while (ecs_query_next(&it)) {
		EcsIdentifier *ident = ecs_field(&it, EcsIdentifier, 1); // self
		EgStrText *path = ecs_field(&it, EgStrText, 2);          // self
		EgStrText *prefix = ecs_field(&it, EgStrText, 3);        // self
		for (int i = 0; i < it.count; ++i, ++ident, ++path, ++prefix) {
			igPushID_Ptr((void*)(intptr_t)it.entities[i]);
			igTableNextColumn();
			igText("%s", ident->value);
			igTableNextColumn();
			text("##path", path);
			igTableNextColumn();
			text("##prefix", prefix);
			igTableNextColumn();
			if (igButton("Export", (ImVec2){0, 0})) {
				export_c(world, q, path->value, prefix->value);
			}
			igPopID();
		}
	}

	igEndTable();
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

ecs_query_t *gui_exporter_query2(ecs_world_t *world)
{
	// clang-format off
	ecs_query_t *q = ecs_query(world, {.filter.terms = {
		{.id = ecs_pair(ecs_id(EcsIdentifier), EcsName)},
		{.first.id = ecs_id(EgStrText), .second.name = "app.exports.path"},
		{.first.id = ecs_id(EgStrText), .second.name = "app.exports.prefix"},
	}});
	// clang-format on

	ecs_iter_t it = ecs_query_iter(world, q);
	while (ecs_query_next(&it)) {
		for (int i = 0; i < it.count; ++i) {
			printf("%s\n", ecs_get_name(world, it.entities[i]));
		}
	}

	return q;
}