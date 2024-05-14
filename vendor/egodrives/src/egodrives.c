#include "egodrives.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <egcan.h>

ECS_COMPONENT_DECLARE(EgOdrivesCmd);

static void Set_Command(ecs_iter_t *it)
{
	EgOdrivesCmd *c = ecs_field(it, EgOdrivesCmd, 1);
	EgCanSignal *s = ecs_field(it, EgCanSignal, 2);
	for (int i = 0; i < it->count; ++i, ++c, ++s) {
		s->canid = (c->nodeid << 5) | (c->cmd);
	}
}

void EgOdrivesImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgOdrives);
	ECS_IMPORT(world, FlecsUnits);
	ECS_IMPORT(world, EgQuantities);
	ecs_set_name_prefix(world, "EgOdrives");

	ECS_COMPONENT_DEFINE(world, EgOdrivesCmd);

	ecs_struct(world,
	{.entity = ecs_id(EgOdrivesCmd),
	.members = {
	{.name = "nodeid", .type = ecs_id(ecs_u32_t)},
	{.name = "cmd", .type = ecs_id(ecs_u32_t)},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = Set_Command,
	.query.filter.terms =
	{
	{.id = ecs_id(EgOdrivesCmd)},
	{.id = ecs_id(EgCanSignal)},
	}});
}