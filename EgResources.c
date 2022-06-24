#include "EgResources.h"
#include "EgQuantities.h"


ECS_COMPONENT_DECLARE(EgImage);
ECS_COMPONENT_DECLARE(EgPath);
//extern ECS_DECLARE(EgPath);


static ECS_COPY(EgPath, dst, src, {
ecs_os_strset((char**)&dst->value, src->value);

})

static ECS_MOVE(EgPath, dst, src, {
ecs_os_free((char*)dst->value);
dst->value = src->value;
src->value = NULL;
})

static ECS_DTOR(EgPath, ptr, {
ecs_os_free((char*)ptr->value);
})


void EgResourcesImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgResources);
	ECS_IMPORT(world, EgQuantities);
	ecs_set_name_prefix(world, "Eg");

	ECS_COMPONENT_DEFINE(world, EgPath);
	//ECS_TAG_DEFINE(world, EgPath);


	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgPath),
	.members = {
	{ .name = "value", .type = ecs_id(ecs_string_t) }
	}
	});

	ecs_set_hooks(world, EgPath, {
	.ctor = ecs_default_ctor,
	.move = ecs_move(EgPath),
	.copy = ecs_copy(EgPath),
	.dtor = ecs_dtor(EgPath)
	});

}
