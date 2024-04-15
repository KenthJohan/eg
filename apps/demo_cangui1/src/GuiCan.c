#include "GuiCan.h"

#include "eg/Components.h"

#include <egcan.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <assert.h>

ECS_COMPONENT_DECLARE(GuiSlider);
ECS_COMPONENT_DECLARE(GuiCanSignalInfo);

typedef struct {
	char const *name;
	EgCanSignal *signal;
	ecs_entity_t e;
} gui_can_table_t;




void gui_can_progress1(ecs_world_t *world, ecs_query_t *q)
{
	assert(world);
	assert(q);
	// int n = ecs_query_entity_count(q);
	gui_can_table_t gui[128] = {0};

	int n = 0;
	ecs_iter_t it = ecs_query_iter(world, q);
	while (ecs_query_next(&it)) {
		GuiSlider *p = ecs_field(&it, GuiSlider, 1);
		EgCanSignal *c = ecs_field(&it, EgCanSignal, 2);
		for (int i = 0; i < it.count; ++i, ++p, ++c) {
			ecs_entity_t e = it.entities[i];
			int list_index = p->list_index;
			if (list_index >= 128) {
				continue;
			}
			char const * name = ecs_get_name(world, e);
			gui[list_index].e = e;
			gui[list_index].name = name;
			gui[list_index].signal = c;
			n = ECS_MAX(list_index, n);
		}
	}



	if (igBeginTable("table_nested1", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable, (ImVec2){0, 0}, 0)) {
		igTableSetupColumn("name", 0, 0, 0);
		igTableSetupColumn("value", 0, 0, 0);
		igTableHeadersRow();

		for (int i = 0; i <= n; ++i) {
			ecs_entity_t e = gui[i].e;
			char const * name = gui[i].name;
			if(name) {
				igPushID_Ptr(gui[i].signal);
				char buf[128];
				snprintf(buf, sizeof(buf), "%s", name);
				igTableNextColumn();
				igText(buf);
				igTableNextColumn();
				int32_t min = 0;
				int32_t max = 255;
				snprintf(buf, sizeof(buf), "##%s", name);
				igPushItemWidth(-1);
				igSliderScalar("", ImGuiDataType_S32, &gui[i].signal->value, &min, &max, "%d", 0);
				igPopItemWidth();
				igPopID();
			} else {
				igTableNextColumn();
				igText("?");
				igTableNextColumn();
				igText("?");
			}
		}


		igEndTable();
	}
}

void GuiCanImport(ecs_world_t *world)
{
	ECS_MODULE(world, GuiCan);
	ECS_IMPORT(world, EgCan);
	ECS_COMPONENT_DEFINE(world, GuiSlider);
	ECS_COMPONENT_DEFINE(world, GuiCanSignalInfo);

	ecs_struct(world,
	{.entity = ecs_id(GuiSlider),
	.members = {
	{.name = "value", .type = ecs_id(ecs_i32_t)},
	{.name = "list_index", .type = ecs_id(ecs_i32_t)},
	}});

	/*
	ecs_system(world, {
	.entity = ecs_entity(world, {
	    .name = "System_GuiBegin",
	    .add = { ecs_dependson(EcsOnUpdate) }
	}),
	.query.filter.terms = {
	    { .id = ecs_id(GuiContext), .src.id = ecs_id(GuiContext) }
	},
	.callback = System_GuiBegin
	});

	ecs_system(world, {
	.entity = ecs_entity(world, {
	    .name = "System_GuiSlider",
	    .add = { ecs_dependson(EcsOnUpdate) }
	}),
	.query.filter.terms = {
	    { .id = ecs_id(GuiContext), .src.id = ecs_id(GuiContext) },
	    { .id = ecs_id(GuiSlider) }
	},
	.callback = System_GuiSlider
	});

	ecs_system(world, {
	.entity = ecs_entity(world, {
	    .name = "System_GuiEnd",
	    .add = { ecs_dependson(EcsOnUpdate) }
	}),
	.query.filter.terms = {
	    { .id = ecs_id(GuiContext), .src.id = ecs_id(GuiContext) }
	},
	.callback = System_GuiEnd
	});
	*/
}