#include "EgAst.h"





ECS_TAG_DECLARE(EgAstIf);
ECS_TAG_DECLARE(EgAstLeft);
ECS_TAG_DECLARE(EgAstRight);
ECS_TAG_DECLARE(EgAstCondition);
ECS_TAG_DECLARE(EgAstContent);
ECS_TAG_DECLARE(EgAstNumber);
ECS_COMPONENT_DECLARE(EgAstCode);


void EgAstImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgAst);
	ecs_set_name_prefix(world, "EgAst");

	ECS_TAG_DEFINE(world, EgAstIf);
	ECS_TAG_DEFINE(world, EgAstLeft);
	ECS_TAG_DEFINE(world, EgAstRight);
	ECS_TAG_DEFINE(world, EgAstCondition);
	ECS_TAG_DEFINE(world, EgAstContent);
	ECS_TAG_DEFINE(world, EgAstNumber);
	ECS_COMPONENT_DEFINE(world, EgAstCode);

	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity = ecs_id(EgAstCode),
	.members = {
	{ .name = "number", .type = ecs_id(ecs_i32_t) }
	}
	});


}


