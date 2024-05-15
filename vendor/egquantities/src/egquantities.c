#include "egquantities.h"


ECS_COMPONENT_DECLARE(EgQuantitiesIsq);
ECS_COMPONENT_DECLARE(EgQuantitiesProgress);
ECS_COMPONENT_DECLARE(EgQuantitiesRangedGeneric);

ECS_TAG_DECLARE(EgQuantitiesVoltage);


void EgQuantitiesImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgQuantities);
	ecs_set_name_prefix(world, "EgQuantities");

	ECS_COMPONENT_DEFINE(world, EgQuantitiesIsq);
	ECS_COMPONENT_DEFINE(world, EgQuantitiesProgress);
	ECS_COMPONENT_DEFINE(world, EgQuantitiesRangedGeneric);
	ECS_TAG_DEFINE(world, EgQuantitiesVoltage);

	ecs_struct(world,
	{.entity = ecs_id(EgQuantitiesIsq),
	.members = {
	{.name = "symbol", .type = ecs_id(ecs_string_t)},
	{.name = "length", .type = ecs_id(ecs_i32_t)},
	{.name = "mass", .type = ecs_id(ecs_i32_t)},
	{.name = "time", .type = ecs_id(ecs_i32_t)},
	{.name = "current", .type = ecs_id(ecs_i32_t)},
	{.name = "temperature", .type = ecs_id(ecs_i32_t)},
	{.name = "substance", .type = ecs_id(ecs_i32_t)},
	{.name = "intensity", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgQuantitiesProgress),
	.members = {
	{.name = "min", .type = ecs_id(ecs_f32_t)},
	{.name = "max", .type = ecs_id(ecs_f32_t)},
	{.name = "value", .type = ecs_id(ecs_f32_t)}
	}});

	// https://github.com/SanderMertens/flecs/blob/37233f127d5006ceb0087fbfcd2f3e36f5b77a23/src/addons/meta/definitions.c#L151
	// https://github.com/SanderMertens/flecs/blob/37233f127d5006ceb0087fbfcd2f3e36f5b77a23/include/flecs/addons/meta.h#L194
    ecs_entity_t primitive_kind = ecs_enum_init(world, &(ecs_enum_desc_t){
        .entity = ecs_entity(world, { .name = "PrimitiveKind" }),
        .constants = {
            { .name = "Bool", 1 }, 
            { .name = "Char" }, 
            { .name = "Byte" }, 
            { .name = "U8" }, 
            { .name = "U16" }, 
            { .name = "U32" }, 
            { .name = "U64 "},
            { .name = "I8" }, 
            { .name = "I16" }, 
            { .name = "I32" }, 
            { .name = "I64" }, 
            { .name = "F32" }, 
            { .name = "F64" }, 
            { .name = "UPtr "},
            { .name = "IPtr" }, 
            { .name = "String" }, 
            { .name = "Entity" },
            { .name = "Id" }
        }
    });


	ecs_struct(world,
	{.entity = ecs_id(EgQuantitiesRangedGeneric),
	.members = {
	{.name = "kind", .type = primitive_kind},
	{.name = "min", .type = ecs_id(ecs_u64_t)},
	{.name = "max", .type = ecs_id(ecs_u64_t)},
	{.name = "rx", .type = ecs_id(ecs_u64_t)},
	{.name = "tx", .type = ecs_id(ecs_u64_t)},

	// TODO: Remove these. This is only used for flecs script to set generic min max:
	{.name = "min_f32", .type = ecs_id(ecs_f32_t)},
	{.name = "max_f32", .type = ecs_id(ecs_f32_t)},
	{.name = "min_f64", .type = ecs_id(ecs_f64_t)},
	{.name = "max_f64", .type = ecs_id(ecs_f64_t)},
	{.name = "min_i64", .type = ecs_id(ecs_i64_t)},
	{.name = "max_i64", .type = ecs_id(ecs_i64_t)},
	{.name = "min_u64", .type = ecs_id(ecs_u64_t)},
	{.name = "max_u64", .type = ecs_id(ecs_u64_t)}
	}});



	ecs_set(world, EgQuantitiesVoltage, EgQuantitiesIsq, {.symbol = "V", .length = 2, .mass = 1, .time = -3, .current = -1});

	



}