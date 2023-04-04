#include "EgQuantities.h"
#include "eg_basics.h"
#include <math.h>
#include <stdio.h>

ECS_DECLARE(EgHover1);
ECS_DECLARE(EgUserinput);
ECS_DECLARE(EgMouse);
ECS_DECLARE(EgPosition);
ECS_DECLARE(EgPositionRelative);
ECS_DECLARE(EgRectangle);
ECS_DECLARE(EgColor);
ECS_COMPONENT_DECLARE(EgV1F32);
ECS_COMPONENT_DECLARE(EgV2F32);
ECS_COMPONENT_DECLARE(EgV3F32);
ECS_COMPONENT_DECLARE(EgV4F32);
ECS_COMPONENT_DECLARE(EgV4U8);
ECS_COMPONENT_DECLARE(EgText);
ECS_COMPONENT_DECLARE(EgKeyboard);

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



void System_Hover1(ecs_iter_t* it)
{
    const EgV2F32 *p   = ecs_field(it, EgV2F32, 1); // position
    const EgV2F32 *r   = ecs_field(it, EgV2F32, 2); // rectangle
	const EgV2F32 *mp0  = ecs_field(it, EgV2F32, 3); // mouse position
    for (int i = 0; i < it->count; i ++)
	{
		EgV2F32 o = {mp0->x - p[i].x, mp0->y - p[i].y};
		int g = ((o.x > 0) && (o.x < r[i].x)) && 
				((o.y > 0) && (o.y < r[i].y));
		if(g){ecs_add(it->world, it->entities[i], EgHover1);}
		else{ecs_remove(it->world, it->entities[i], EgHover1);}
		
    }
}








void EgQuantitiesImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgQuantities);
	ecs_set_name_prefix(world, "Eg");



	ECS_TAG_DEFINE(world, EgHover1);
	ECS_TAG_DEFINE(world, EgUserinput);
	ECS_TAG_DEFINE(world, EgMouse);
	ECS_TAG_DEFINE(world, EgPosition);
	ECS_TAG_DEFINE(world, EgPositionRelative);
	ECS_TAG_DEFINE(world, EgRectangle);
	ECS_TAG_DEFINE(world, EgColor);
	ECS_COMPONENT_DEFINE(world, EgV1F32);
	ECS_COMPONENT_DEFINE(world, EgV2F32);
	ECS_COMPONENT_DEFINE(world, EgV3F32);
	ECS_COMPONENT_DEFINE(world, EgV4F32);
	ECS_COMPONENT_DEFINE(world, EgV4U8);
	ECS_COMPONENT_DEFINE(world, EgText);
	ECS_COMPONENT_DEFINE(world, EgKeyboard);

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

	ecs_struct(world, {
	.entity = ecs_id(EgV4U8),
	.members = {
	{ .name = "x", .type = ecs_id(ecs_u8_t) },
	{ .name = "y", .type = ecs_id(ecs_u8_t) },
	{ .name = "z", .type = ecs_id(ecs_u8_t) },
	{ .name = "w", .type = ecs_id(ecs_u8_t) }
	}
	});

	ecs_struct(world, {
	.entity = ecs_id(EgText),
	.members = {
	{ .name = "value", .type = ecs_id(ecs_string_t) }
	}
	});

	ecs_struct(world, {
	.entity = ecs_id(EgKeyboard),
	.members = {
	{ .name = "keys", .type = ecs_id(ecs_u8_t), .count = EG_KEYBOARD_SIZE }
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




    ecs_entity_t e_System_Hover1 = ecs_system(world, {
        .entity = ecs_entity(world, {
			.name = "System_Hover1",
			.add = { ecs_dependson(EcsOnUpdate) }
		}),
        .query.filter.terms = {
            { .id = ecs_pair(ecs_id(EgV2F32), EgPosition), .inout = EcsIn },
            { .id = ecs_pair(ecs_id(EgV2F32), EgRectangle), .inout = EcsIn },
            { .id = ecs_pair(ecs_id(EgV2F32), EgPosition), .src.id = ecs_id(EgMouse) },
        },
        .callback = System_Hover1
    });









}
