#include "CanDbc.h"

ECS_COMPONENT_DECLARE(CanDbcSignal);
ECS_COMPONENT_DECLARE(CanDbcFlags);

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
	ecs_set_name_prefix(world, "CanDbc");
	ECS_COMPONENT_DEFINE(world, CanDbcSignal);
	ECS_COMPONENT_DEFINE(world, CanDbcFlags);

    ecs_bitmask(world,{
        .entity = ecs_id(CanDbcFlags), // Make sure to use existing id
        .constants = {
            { .name = "Bacon", .value = CanDbcFlagsBacon },
            { .name = "Lettuce", .value = CanDbcFlagsLettuce },
            { .name = "Tomato", .value = CanDbcFlagsTomato }
        }
    });

    ecs_struct(world, {
        .entity = ecs_id(CanDbcSignal),
        .members = {
            { .name = "x", .type = ecs_id(ecs_f32_t) },
            { .name = "y", .type = ecs_id(ecs_f32_t) }
        }
    });

}