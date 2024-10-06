#include "CanDbc.h"

ECS_COMPONENT_DECLARE(CanDbcSignal);

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
	ECS_COMPONENT_DEFINE(world, CanDbcSignal);

    // Add reflection data to component
	/*
	char name[128];
	int type;
	int order;
	int mode;
	int start;
	int length;
	double factor;
	double offset;
	double min;
	double max;
	char unit[128];
	*/
    ecs_struct(world, {
        .entity = ecs_id(CanDbcSignal),
        .members = {
            { .name = "x", .type = ecs_id(ecs_f32_t) },
            { .name = "y", .type = ecs_id(ecs_f32_t) }
        }
    });

}