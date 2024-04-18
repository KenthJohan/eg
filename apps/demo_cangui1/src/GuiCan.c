#include "GuiCan.h"

#include "eg/Components.h"

#include <egcan.h>
#include <egquantities.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <assert.h>

ECS_COMPONENT_DECLARE(GuiSlider);
ECS_COMPONENT_DECLARE(GuiCanSignalInfo);

typedef struct {
	char const *name;
	EgCanBusDescription *desc;
	EgCanBus * bus;
	EgCanSignal *signal;
	EgQuantitiesIsq *q;
	EgCanBusBook * book;
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
		EgCanBus *bus = ecs_field(&it, EgCanBus, 1);
		EgCanBusDescription *desc = ecs_field(&it, EgCanBusDescription, 2);
		EgCanBusBook *book = ecs_field(&it, EgCanBusBook, 3);
		GuiSlider *slider = ecs_field(&it, GuiSlider, 4);
		EgCanSignal *signal = ecs_field(&it, EgCanSignal, 5);
		EgQuantitiesIsq *quant = ecs_field(&it, EgQuantitiesIsq, 6);
		for (int i = 0; i < it.count; ++i, ++slider, ++signal, ++quant) {
			ecs_entity_t e = it.entities[i];
			int list_index = slider->list_index;
			if (list_index >= 128) {
				continue;
			}
			char const * name = ecs_get_name(world, e);
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



	if (igBeginTable("Signals table", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable, (ImVec2){0, 0}, 0)) {
		igTableSetupColumn("name", 0, 0, 0);
		igTableSetupColumn("bus", 0, 0, 0);
		igTableSetupColumn("sock", 0, 0, 0);
		igTableSetupColumn("value", 0, 0, 0);
		igTableSetupColumn("q", 0, 0, 0);
		igTableSetupColumn("u", 0, 0, 0);
		igTableHeadersRow();

		for (int i = 0; i <= n; ++i) {
			ecs_entity_t e = gui[i].e;
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
				int32_t min = 0;
				int32_t max = 255;
				igPushItemWidth(-1);
				if (igSliderScalar("", ImGuiDataType_S32, &signal->value, &min, &max, "%d", 0)) {
					EgCanBusBook_prepare_send(book, signal);
				};
				igPopItemWidth();
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