#include "EgTypes.h"

ECS_COMPONENT_DECLARE(EgName);
ECS_COMPONENT_DECLARE(EgVersion);
ECS_COMPONENT_DECLARE(EgIndex);

ECS_DECLARE(EgApplication);
ECS_DECLARE(EgEngine);
ECS_DECLARE(EgApi);


static ECS_COPY(EgName, dst, src, {
ecs_os_strset((char**)&dst->value, src->value);
})

static ECS_MOVE(EgName, dst, src, {
ecs_os_free((char*)dst->value);
dst->value = src->value;
src->value = NULL;
})

static ECS_DTOR(EgName, ptr, {
ecs_os_free((char*)ptr->value);
})



void EgTypesImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgTypes);
	ecs_set_name_prefix(world, "Eg");

	ECS_TAG_DEFINE(world, EgApplication);
	ECS_TAG_DEFINE(world, EgEngine);
	ECS_TAG_DEFINE(world, EgApi);

	ECS_COMPONENT_DEFINE(world, EgName);
	ECS_COMPONENT_DEFINE(world, EgVersion);
	ECS_COMPONENT_DEFINE(world, EgIndex);

	ecs_struct_init(world, &(ecs_struct_desc_t){
	.entity = ecs_entity(world, {.id = ecs_id(EgIndex)}),
	.members = {
	{ .name = "index", .type = ecs_id(ecs_u32_t) }
	}
	});

	ecs_struct_init(world, &(ecs_struct_desc_t){
						.entity = ecs_entity(world, {.id = ecs_id(EgName)}),
	.members = {
	{ .name = "value", .type = ecs_id(ecs_string_t) }
	}
	});

	ecs_struct_init(world, &(ecs_struct_desc_t){
						.entity = ecs_entity(world, {.id = ecs_id(EgVersion)}),

	.members = {
	{ .name = "version", .type = ecs_id(ecs_u32_t) }
	}
	});

	ecs_set_hooks(world, EgName, {
	.ctor = ecs_default_ctor,
	.move = ecs_move(EgName),
	.copy = ecs_copy(EgName),
	.dtor = ecs_dtor(EgName)
	});

}




