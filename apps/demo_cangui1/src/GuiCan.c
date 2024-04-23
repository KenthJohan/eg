#include "GuiCan.h"

#include "eg/Components.h"

#include <egcan.h>
#include <egquantities.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <assert.h>

ECS_COMPONENT_DECLARE(GuiSlider);
ECS_COMPONENT_DECLARE(GuiCanSignalInfo);



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