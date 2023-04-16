#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "flecs.h"



typedef struct{
	ecs_i32_t value;
} Price;

typedef struct{
	ecs_i32_t value;
} Mass;

int main (int argc, char * argv []){
	ecs_world_t *world = ecs_init_w_args(argc, argv);
	ECS_IMPORT(world, FlecsUnits);
	ECS_COMPONENT(world, Price);
	ECS_COMPONENT(world, Mass);
	ecs_struct(world, {
	.entity = ecs_id(Price),
	.members = {
	{ .name = "value", .type = ecs_id(ecs_i32_t) }
	}
	});
	ecs_struct(world, {
	.entity = ecs_id(Mass),
	.members = {
	{ .name = "value", .type = ecs_id(ecs_i32_t) }
	}
	});
	ecs_entity_t A = ecs_new_entity(world, "A");
	ecs_entity_t B1 = ecs_new_entity(world, "B1");
	ecs_entity_t B2 = ecs_new_entity(world, "B2");
	ecs_entity_t C = ecs_new_entity(world, "C");
	ecs_set(world, A, Price, {10});
	ecs_set(world, A, Mass, {1});
	//ecs_set(world, B1, Mass, {2});
	//ecs_set(world, B2, Mass, {3});
	ecs_add_pair(world, B1, EcsIsA, A);
	ecs_add_pair(world, B2, EcsIsA, A);
	ecs_add_pair(world, C, EcsIsA, B1);
	ecs_add_pair(world, C, EcsIsA, B2);
	//assert(ecs_get(world, C, Price)->value == 10);
	//assert(ecs_get(world, C, Mass)->value == 2);
	//https://www.flecs.dev/explorer/?remote=true
	ecs_singleton_set(world, EcsRest, {0});
	return ecs_app_run(world, &(ecs_app_desc_t){
	.enable_rest = true
	});
}
