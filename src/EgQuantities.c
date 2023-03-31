#include "EgQuantities.h"
#include "eg_basics.h"
#include <math.h>
#include <stdio.h>

ECS_DECLARE(EgPosition);
ECS_DECLARE(EgPositionRelative);
ECS_DECLARE(EgRectangle);
ECS_COMPONENT_DECLARE(EgV1F32);
ECS_COMPONENT_DECLARE(EgV2F32);
ECS_COMPONENT_DECLARE(EgV3F32);
ECS_COMPONENT_DECLARE(EgV4F32);
ECS_COMPONENT_DECLARE(EgText);

static ECS_COPY(EgText, dst, src, {
ecs_os_strset((char**)&dst->value, src->value);
})

static ECS_MOVE(EgText, dst, src, {
ecs_os_free((char*)dst->value);
dst->value = src->value;
src->value = NULL;
})

static ECS_DTOR(EgText, ptr, {
ecs_os_free((char*)ptr->value);
})


void Move(ecs_iter_t *it)
{
    const EgV2F32 *r  = ecs_field(it, EgV2F32, 1);
          EgV2F32 *p  = ecs_field(it, EgV2F32, 2);
    const EgV2F32 *p0 = ecs_field(it, EgV2F32, 3);

    for (int i = 0; i < it->count; i ++)
	{
        p[i].x = p0->x + r[i].x;
        p[i].y = p0->y + r[i].y;
        //printf("%s: {%f, %f}\n", ecs_get_name(it->world, it->entities[i]), p[i].x, p[i].y);
        //printf("%s:\n", ecs_get_name(it->world, it->entities[i]));
    }
}



void EgQuantitiesImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgQuantities);
	ecs_set_name_prefix(world, "Eg");


	ECS_TAG_DEFINE(world, EgPosition);
	ECS_TAG_DEFINE(world, EgPositionRelative);
	ECS_TAG_DEFINE(world, EgRectangle);
	ECS_COMPONENT_DEFINE(world, EgV1F32);
	ECS_COMPONENT_DEFINE(world, EgV2F32);
	ECS_COMPONENT_DEFINE(world, EgV3F32);
	ECS_COMPONENT_DEFINE(world, EgV4F32);
	ECS_COMPONENT_DEFINE(world, EgText);

	ecs_struct(world, {
	.entity = ecs_id(EgV1F32),
	.members = {
	{ .name = "x", .type = ecs_id(ecs_f32_t) }
	}
	});

	ecs_struct(world, {
	.entity = ecs_id(EgV2F32),
	.members = {
	{ .name = "x", .type = ecs_id(ecs_f32_t) },
	{ .name = "y", .type = ecs_id(ecs_f32_t) }
	}
	});

	ecs_struct(world, {
	.entity = ecs_id(EgV3F32),
	.members = {
	{ .name = "x", .type = ecs_id(ecs_f32_t) },
	{ .name = "y", .type = ecs_id(ecs_f32_t) },
	{ .name = "z", .type = ecs_id(ecs_f32_t) }
	}
	});

	ecs_struct(world, {
	.entity = ecs_id(EgV4F32),
	.members = {
	{ .name = "x", .type = ecs_id(ecs_f32_t) },
	{ .name = "y", .type = ecs_id(ecs_f32_t) },
	{ .name = "z", .type = ecs_id(ecs_f32_t) },
	{ .name = "w", .type = ecs_id(ecs_f32_t) }
	}
	});


	ecs_set_hooks(world, EgText, {
	.ctor = ecs_default_ctor,
	.move = ecs_move(EgText),
	.copy = ecs_copy(EgText),
	.dtor = ecs_dtor(EgText)
	});




    ecs_entity_t move = ecs_system(world, {
        .entity = ecs_entity(world, {
			.name = "Move",
			.add = { ecs_dependson(EcsOnUpdate) }
		}),
        .query.filter.terms = {
            { .id = ecs_pair(ecs_id(EgV2F32), EgPositionRelative), .inout = EcsIn },
            { .id = ecs_pair(ecs_id(EgV2F32), EgPosition), .inout = EcsOut },
            {
                .id = ecs_pair(ecs_id(EgV2F32), EgPosition), 
                .inout = EcsIn,
                // Get from the parent, in breadth-first order (cascade)
                .src.flags = EcsParent | EcsCascade,
                // Make parent term optional so we also match the root (sun)
                .oper = EcsOptional
            }
        },
        .callback = Move
    });














}
