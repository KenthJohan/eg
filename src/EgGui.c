#include "EgGui.h"
#include "EgQuantities.h"
#include "EgUserinput.h"
#include "eg_basics.h"
#include <math.h>
#include <stdio.h>

ECS_DECLARE(EgHover1);
ECS_COMPONENT_DECLARE(EgMargin4);
ECS_COMPONENT_DECLARE(EgZIndex);
ECS_COMPONENT_DECLARE(EgHover);


void System_Hover1(ecs_iter_t* it)
{
    const EgZIndex *z  = ecs_field(it, EgZIndex, 1); // zindex
    const EgV2F32 *p   = ecs_field(it, EgV2F32, 2); // position
    const EgV2F32 *r   = ecs_field(it, EgV2F32, 3); // rectangle
	const EgV2F32 *mp0 = ecs_field(it, EgV2F32, 4); // mouse position
	EgHover *h0 = ecs_field(it, EgHover, 5); // mouse
	//ecs_entity_t e_mp0 = ecs_field_src(it, 3);
    for (int i = 0; i < it->count; i ++)
	{
		EgV2F32 o = {mp0->x - p[i].x, mp0->y - p[i].y};
		int g = ((o.x > 0) && (o.x < r[i].x)) && ((o.y > 0) && (o.y < r[i].y));
		if(g)
		{
			if(z[i].z >= h0->zindex)
			{
				h0->entity = it->entities[i];
				h0->zindex = z[i].z;
			}
		}
		else
		{
			if(h0->entity == it->entities[i])
			{
				h0->entity = 0;
				h0->zindex = 0;
			}
		}
    }
}

void System_Follow_Mouse(ecs_iter_t* it)
{
    EgV2F32 *p   = ecs_field(it, EgV2F32, 1); // position
	const EgV2F32 *mp0  = ecs_field(it, EgV2F32, 2); // mouse velocity
	const EgMouse *ms0  = ecs_field(it, EgMouse, 3); // mouse state
	const EgHover *h0  = ecs_field(it, EgHover, 4); // mouse state
	if(ms0->left == 0) {return;}
    for (int i = 0; i < it->count; i ++)
	{
		if(h0->entity == it->entities[i])
		{
			p[i].x += mp0->x;
			p[i].y += mp0->y;
		}
    }
}

void System_Margin(ecs_iter_t *it)
{
    EgV2F32 *p  = ecs_field(it, EgV2F32, 1);
    EgV2F32 *r  = ecs_field(it, EgV2F32, 2);
    EgMargin4 *m  = ecs_field(it, EgMargin4, 3);
    EgV2F32 *p0 = ecs_field(it, EgV2F32, 4);
    EgV2F32 *r0 = ecs_field(it, EgV2F32, 5);
    for (int i = 0; i < it->count; i ++)
	{
        p[i].x = p0->x + m->left;
        p[i].y = p0->y + m->top;
		r[i].x = r0[0].x - (m->left + m->right);
		r[i].y = r0[0].y - (m->botton + m->top);
    }
}

void System_ZIndex(ecs_iter_t *it)
{
    EgZIndex *z  = ecs_field(it, EgZIndex, 1);
    EgZIndex *z0  = ecs_field(it, EgZIndex, 2);
	ecs_entity_t e_z0 = ecs_field_src(it, 2);
	if(z0 == NULL){return;}
    for (int i = 0; i < it->count; i ++)
	{
        //printf("%s:%i, %s:%i\n", ecs_get_name(it->world, e_z0), z0->z, ecs_get_name(it->world, it->entities[i]), z[i].z);
		z[i].z = z0->z + 1;
    }
}

void EgGuiImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgGui);
	ecs_set_name_prefix(world, "Eg");
	ECS_IMPORT(world, EgQuantities);
	ECS_IMPORT(world, EgUserinput);

	ECS_TAG_DEFINE(world, EgHover1);
	ECS_COMPONENT_DEFINE(world, EgMargin4);
	ECS_COMPONENT_DEFINE(world, EgZIndex);
	ECS_COMPONENT_DEFINE(world, EgHover);

	ecs_struct(world, {
	.entity = ecs_id(EgZIndex),
	.members = {
	{ .name = "z", .type = ecs_id(ecs_u8_t) },
	}
	});

	ecs_struct(world, {
	.entity = ecs_id(EgMargin4),
	.members = {
	{ .name = "left", .type = ecs_id(ecs_f32_t) },
	{ .name = "right", .type = ecs_id(ecs_f32_t) },
	{ .name = "botton", .type = ecs_id(ecs_f32_t) },
	{ .name = "top", .type = ecs_id(ecs_f32_t) }
	}
	});

	ecs_struct(world, {
	.entity = ecs_id(EgHover),
	.members = {
	{ .name = "entity", .type = ecs_id(ecs_entity_t) },
	{ .name = "zindex", .type = ecs_id(ecs_u32_t) }
	}
	});






    ecs_entity_t e_System_Hover1 = ecs_system(world, {
        .entity = ecs_entity(world, {
			.name = "System_Hover1",
			.add = { ecs_dependson(EcsOnUpdate) }
		}),
        .query.filter.terms = {
            { .id = ecs_id(EgZIndex), .inout = EcsIn },
            { .id = ecs_pair(ecs_id(EgV2F32), EgPosition), .inout = EcsIn },
            { .id = ecs_pair(ecs_id(EgV2F32), EgRectangle), .inout = EcsIn },
            { .id = ecs_pair(ecs_id(EgV2F32), EgPosition), .src.id = ecs_id(EgMouse) },
            { .id = ecs_id(EgHover), .src.id = ecs_id(EgMouse) },
        },
        .callback = System_Hover1
    });

    ecs_entity_t e_System_Follow_Mouse = ecs_system(world, {
        .entity = ecs_entity(world, {
			.name = "System_Follow_Mouse",
			.add = { ecs_dependson(EcsOnUpdate) }
		}),
        .query.filter.terms = {
            { .id = ecs_pair(ecs_id(EgV2F32), EgPosition), .inout = EcsIn },
            { .id = ecs_pair(ecs_id(EgV2F32), EgVelocity), .src.id = ecs_id(EgMouse) },
            { .id = ecs_id(EgMouse), .src.id = ecs_id(EgMouse) },
            { .id = ecs_id(EgHover), .src.id = ecs_id(EgMouse) },
        },
        .callback = System_Follow_Mouse
    });


    ecs_entity_t e_System_Margin = ecs_system(world, {
        .entity = ecs_entity(world, {
			.name = "System_Margin",
			.add = { ecs_dependson(EcsOnUpdate) }
		}),
        .query.filter.terms = {
            {.id = ecs_pair(ecs_id(EgV2F32), EgPosition), .inout = EcsOut },
            {.id = ecs_pair(ecs_id(EgV2F32), EgRectangle), .inout = EcsOut },
            {.id = ecs_id(EgMargin4), .inout = EcsIn },
            {.id = ecs_pair(ecs_id(EgV2F32), EgPosition), .inout = EcsIn,.src.flags = EcsParent | EcsCascade,.oper = EcsOptional},
            {.id = ecs_pair(ecs_id(EgV2F32), EgRectangle), .inout = EcsIn,.src.flags = EcsParent,.oper = EcsOptional}
        },
        .callback = System_Margin
    });

    ecs_entity_t e_System_ZIndex = ecs_system(world, {
        .entity = ecs_entity(world, {
			.name = "System_ZIndex",
			.add = { ecs_dependson(EcsOnUpdate) }
		}),
        .query.filter.terms = {
            {.id = ecs_id(EgZIndex), .inout = EcsOut },
            {.id = ecs_id(EgZIndex), .inout = EcsIn, .src.flags = EcsParent | EcsCascade},
        },
        .callback = System_ZIndex
    });


}
