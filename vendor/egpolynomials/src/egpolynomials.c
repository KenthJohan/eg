#include "egpolynomials.h"

ECS_COMPONENT_DECLARE(EgPolynomialsLinear);


void EgPolynomialsImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgPolynomials);
	ecs_set_name_prefix(world, "EgPolynomials");

	ECS_COMPONENT_DEFINE(world, EgPolynomialsLinear);

	ecs_struct(world,
	{.entity = ecs_id(EgPolynomialsLinear),
	.members = {
	{.name = "k0", .type = ecs_id(ecs_f64_t)},
	{.name = "k1", .type = ecs_id(ecs_f64_t)},
	}});

}