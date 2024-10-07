#include "CanDbc.h"

#include <egpolynomials.h>

ECS_COMPONENT_DECLARE(CanDbcSignal);
ECS_COMPONENT_DECLARE(CanDbcType);
ECS_COMPONENT_DECLARE(CanDbcOrder);

int dbcsig_meta_bitpos_to_signal(CanDbcSignal metas[], int length, int bitpos)
{
	for (int i = 0; i < length; ++i) {
		if (metas[i].start > bitpos) {
			continue;
		}
		if ((metas[i].start + metas[i].length) <= bitpos) {
			continue;
		}
		return i;
	}
	return -1;
}

void CanDbcImport(ecs_world_t *world)
{
	ECS_MODULE(world, CanDbc);
	ECS_IMPORT(world, EgPolynomials);

	ecs_set_name_prefix(world, "CanDbc");
	ECS_COMPONENT_DEFINE(world, CanDbcSignal);
	ECS_COMPONENT_DEFINE(world, CanDbcType);
	ECS_COMPONENT_DEFINE(world, CanDbcOrder);

	ecs_struct(world, {
		.entity = ecs_id(CanDbcSignal),
		.members = {
			{ .name = "x", .type = ecs_id(ecs_f32_t) },
			{ .name = "y", .type = ecs_id(ecs_f32_t) }
		}
	});

	ecs_enum(world, {.entity = ecs_id(CanDbcType), .constants = {
		{.name = "Unsigned" , .value = CanDbcTypeUnsigned   },
		{.name = "Signed"   , .value = CanDbcTypeSigned },
		}});

	ecs_enum(world, {.entity = ecs_id(CanDbcOrder), .constants = {
		{.name = "BigEndian"   , .value = CanDbcOrderBigEndian   },
		{.name = "LitleEndian" , .value = CanDbcOrderLitleEndian },
		}});
		
}