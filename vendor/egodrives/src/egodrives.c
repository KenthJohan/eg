#include "egodrives.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <egcan.h>

ECS_COMPONENT_DECLARE(EgOdrivesCmd);
ECS_COMPONENT_DECLARE(eg_odrive_input_mode_t);

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
	ECS_COMPONENT_DEFINE(world, eg_odrive_input_mode_t);

	/*
	
	    ecs_enum(world, {
        .entity = ecs_id(eg_odrive_input_mode_t),
        .constants = {
            { .name = "INACTIVE", .value = EG_ODRIVE_INPUT_MODE_INACTIVE },
            { .name = "PASSTHROUGH", .value = EG_ODRIVE_INPUT_MODE_PASSTHROUGH },
            { .name = "VEL_RAMP", .value = EG_ODRIVE_INPUT_MODE_VEL_RAMP },
            { .name = "POS_FILTER", .value = EG_ODRIVE_INPUT_MODE_POS_FILTER },
            { .name = "MIX_CHANNELS", .value = EG_ODRIVE_INPUT_MODE_MIX_CHANNELS },
            { .name = "TRAP_TRAJ", .value = EG_ODRIVE_INPUT_MODE_TRAP_TRAJ },
            { .name = "TORQUE_RAMP", .value = EG_ODRIVE_INPUT_MODE_TORQUE_RAMP },
            { .name = "MIRROR", .value = EG_ODRIVE_INPUT_MODE_MIRROR },
            { .name = "TUNING", .value = EG_ODRIVE_INPUT_MODE_TUNING }
        }
    });

    ecs_entity_t ent = ecs_new_entity(world, "HelloTest1");
    ecs_set(world, ent, eg_odrive_input_mode_t, {EG_ODRIVE_INPUT_MODE_PASSTHROUGH});
	*/

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