#include "EgPhysics.h"
#include <ecsx.h>

ECS_COMPONENT_DECLARE(EgPhysicsWorldDef);
ECS_COMPONENT_DECLARE(EgPhysicsBodyDef);
ECS_COMPONENT_DECLARE(EgPhysicsShapesDef);
ECS_COMPONENT_DECLARE(EgPhysicsJointDef);
ECS_COMPONENT_DECLARE(EgPhysicsJointApplyRule);
ECS_COMPONENT_DECLARE(EgPhysicsOverlapChecking);
ECS_TAG_DECLARE(EgPhysicsTargetTransform);

void MyMonitor(ecs_iter_t *it)
{
	EgPhysicsJointApplyRule *rule = ecs_field_shared(it, EgPhysicsJointApplyRule, 0);
	EgPhysicsJointDef       *def  = ecs_field_shared(it, EgPhysicsJointDef, 1);
	if (it->event == EcsOnAdd) {
		for (int i = 0; i < it->count; i++) {
			char const *entity_name = ecs_get_name(it->world, it->entities[i]);
			printf("Entity %llu (%s) started matching query\n", (unsigned long long)it->entities[i], entity_name);
			// Entity started matching query
			ecs_entity_t joint = ecs_new(it->world);
			ecs_set_name(it->world, joint, "joint");
			ecs_set_id(it->world, joint, ecs_pair(ecs_id(EgPhysicsJointDef), it->entities[i]), sizeof(EgPhysicsJointDef), def);
			ecs_add_pair(it->world, joint, EcsChildOf, rule->body_a);
		}
	} else if (it->event == EcsOnRemove) {
		for (int i = 0; i < it->count; i++) {
			char const *entity_name = ecs_get_name(it->world, it->entities[i]);
			printf("Entity %llu (%s) stopped matching query\n", (unsigned long long)it->entities[i], entity_name);
			ecs_entity_t joint = ecs_lookup_child(it->world, rule->body_a, "joint");
			if (joint) {
				printf("Deleting joint entity %llu (%s)\n", (unsigned long long)joint, ecs_get_name(it->world, joint));
				ecs_delete(it->world, joint);
			}
		}
	}
}

static void Observer_Joint_Apply_Rule(ecs_iter_t *it)
{
	EgPhysicsJointApplyRule *rule = ecs_field(it, EgPhysicsJointApplyRule, 0);
	for (int i = 0; i < it->count; ++i, ++rule) {
		ecs_observer(it->world,
		{
		.query.terms = {
		{.id = ecs_id(EgPhysicsJointApplyRule), .src.id = it->entities[i], .inout = EcsInOutFilter},
		{.id = ecs_id(EgPhysicsJointDef), .src.id = it->entities[i], .inout = EcsInOutFilter},
		{.id = rule->body_b_filter},
		},
		.events   = {EcsMonitor},
		.callback = MyMonitor,
		});
	}
}

void EgPhysicsImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgPhysics);
	ecs_set_name_prefix(world, "EgPhysics");

	ECS_COMPONENT_DEFINE(world, EgPhysicsWorldDef);
	ECS_COMPONENT_DEFINE(world, EgPhysicsBodyDef);
	ECS_COMPONENT_DEFINE(world, EgPhysicsShapesDef);
	ECS_COMPONENT_DEFINE(world, EgPhysicsJointDef);
	ECS_TAG_DEFINE(world, EgPhysicsTargetTransform);
	ecs_add_id(world, EgPhysicsTargetTransform, EcsTraversable);
	ECS_COMPONENT_DEFINE(world, EgPhysicsOverlapChecking);
	ecs_add_id(world, ecs_id(EgPhysicsOverlapChecking), EcsTraversable);
	ECS_COMPONENT_DEFINE(world, EgPhysicsJointApplyRule);

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgPhysicsWorldDef),
	.members = {
	{.name = "gravity_x", .type = ecs_id(ecs_f32_t)},
	{.name = "gravity_y", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgPhysicsBodyDef),
	.members = {
	{.name = "type", .type = ecs_id(ecs_i32_t)},
	}});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgPhysicsShapesDef),
	.members = {
	{.name = "density", .type = ecs_id(ecs_f32_t)},
	{.name = "friction", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgPhysicsJointDef),
	.members = {
	{.name = "linear_hertz", .type = ecs_id(ecs_f32_t)},
	{.name = "linear_damping", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgPhysicsJointApplyRule),
	.members = {
	{.name = "body_a", .type = ecs_id(ecs_entity_t)},
	{.name = "body_b_filter", .type = ecs_id(ecs_entity_t)},
	}});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgPhysicsOverlapChecking),
	.members = {
	{.name = "tag", .type = ecs_id(ecs_entity_t)},
	}});

	ecs_observer(world,
	{.query.terms = {
	 {.id = ecs_id(EgPhysicsJointApplyRule)},
	 },
	.events   = {EcsOnSet},
	.callback = Observer_Joint_Apply_Rule});
}
