#include "EgGui.h"
#include "EgQuantities.h"
#include "EgUserinput.h"
#include "eg_basics.h"
#include <math.h>
#include <stdio.h>

ECS_DECLARE(EgGuiMouseOver);
ECS_DECLARE(EgGuiMouseOver1);
ECS_DECLARE(EgGuiDragging);
ECS_COMPONENT_DECLARE(EgMargin4);
ECS_COMPONENT_DECLARE(EgZIndex);
ECS_COMPONENT_DECLARE(EgHover);
ECS_COMPONENT_DECLARE(EgGuiDrag);
ECS_COMPONENT_DECLARE(EgGuiBorder4);




void System_Hover1(ecs_iter_t* it)
{
	const EgZIndex *z   = ecs_field(it, EgZIndex, 1); // [in] GUI Element Zindex
	const EgV2F32  *p   = ecs_field(it, EgV2F32,  2); // [in] GUI Element Position
	const EgV2F32  *r   = ecs_field(it, EgV2F32,  3); // [in] GUI Element Rectangle
	const EgV2F32  *mp0 = ecs_field(it, EgV2F32,  4); // [in] Userinput Mouse Position
	      EgHover  *h0  = ecs_field(it, EgHover,  5); // [inout] Hover entity 
	      EgV2F32  *hp0 = ecs_field(it, EgV2F32,  6); // [out] Hover relative mouse position 
	//ecs_entity_t e_mp0 = ecs_field_src(it, 3);
    for (int i = 0; i < it->count; i ++)
	{
		ecs_remove(it->world, it->entities[i], EgGuiMouseOver);
		ecs_remove(it->world, it->entities[i], EgGuiMouseOver1);
		//ecs_remove_id(it->world, it->entities[i], h0->entity1);
		// Check if mouse position is inside the rectangle:
		hp0->x = mp0->x - p[i].x;
		hp0->y = mp0->y - p[i].y;
		int hit = ((hp0->x > 0) && (hp0->x < r[i].x)) && ((hp0->y > 0) && (hp0->y < r[i].y));
		if(hit)
		{
			//printf("Hover: %i\n", it->entities[i]);
			ecs_add(it->world, it->entities[i], EgGuiMouseOver);
			//ecs_add_id(it->world, it->entities[i], h0->entity1); // Does not add assembly proberly
			if(z[i].z >= h0->zindex)
			{
				ecs_add(it->world, it->entities[i], EgGuiMouseOver1);
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

void System_Drag1(ecs_iter_t* it)
{
	const EgV2F32   *mp0  = ecs_field(it, EgV2F32,   1); // [in] Userinput Mouse Position
	const EgMouse   *ms0  = ecs_field(it, EgMouse,   2); // [in] Userinput Mouse State
	      EgHover   *h0   = ecs_field(it, EgHover,   3); // [inout] Hover entity
	const EgV2F32   *hp0  = ecs_field(it, EgV2F32,   4); // [in] Hover position relative
	      EgGuiDrag *drag = ecs_field(it, EgGuiDrag, 5); //
	      EgV2F32   *dp0  = ecs_field(it, EgV2F32,   6); //

	ecs_assert(mp0 != NULL, ECS_INVALID_PARAMETER, NULL);
	ecs_assert(ms0 != NULL, ECS_INVALID_PARAMETER, NULL);
	ecs_assert(h0 != NULL, ECS_INVALID_PARAMETER, NULL);
	ecs_assert(hp0 != NULL, ECS_INVALID_PARAMETER, NULL);
	ecs_assert(drag != NULL, ECS_INVALID_PARAMETER, NULL);
	ecs_assert(dp0 != NULL, ECS_INVALID_PARAMETER, NULL);

	if (ms0->left == EG_EDGE_RISING && h0->entity)
	{
		printf("Drag start: %i\n", h0->entity);
		h0->zindex = 0;
		drag->entity = h0->entity;
		ecs_remove(it->world, drag->entity, EgPosition_V2F32);
		ecs_remove_pair(it->world, drag->entity, EcsChildOf, EcsWildcard);
		ecs_add(it->world, drag->entity, EgGuiDragging);
		(*dp0) = (*hp0);
	}
	if (ms0->left == EG_EDGE_FALLING && drag->entity)
	{
		printf("Drag end: %i %i\n", drag->entity, h0->entity);
		ecs_remove(it->world, drag->entity, EgGuiDragging);
		if(h0->entity && (drag->entity != h0->entity))
		{
			ecs_set(it->world, drag->entity, EgPosition_V2F32, {5,5});
			// Don't add it self as child:
			ecs_add_pair(it->world, drag->entity, EcsChildOf, h0->entity);
		}
		drag->entity = 0;
		//(*dp0) = (EgV2F32){0,0};
	}
	if(drag->entity)
	{
		//ecs_set(it->world, drag->entity, EgPosition_V2F32, {mp0->x-10, mp0->y-10});
	}

}


void System_Margin(ecs_iter_t *it)
{
    EgV2F32   *p  = ecs_field(it, EgV2F32,   1); // GUI Element Position
    EgV2F32   *r  = ecs_field(it, EgV2F32,   2); // GUI Element Rectangle
    EgMargin4 *m  = ecs_field(it, EgMargin4, 3);
    EgV2F32   *r0 = ecs_field(it, EgV2F32,   4); // GUI Element Rectangle Parent
	if(r0 == NULL){return;}
    for (int i = 0; i < it->count; i ++)
	{
		p[i].x = m->left;
		p[i].y = m->top;
		r[i].x = r0[0].x - (m->left + m->right);
		r[i].y = r0[0].y - (m->botton + m->top);
	}
}

void System_ZIndex(ecs_iter_t *it)
{
	EgZIndex *z = ecs_field(it, EgZIndex, 1);
	EgZIndex *z0 = ecs_field(it, EgZIndex, 2);
	ecs_entity_t e_z0 = ecs_field_src(it, 2);
	if(z0 == NULL){return;}
	for (int i = 0; i < it->count; i ++)
	{
		//printf("%s:%i, %s:%i\n", ecs_get_name(it->world, e_z0), z0->z, ecs_get_name(it->world, it->entities[i]), z[i].z);
		z[i].z = z0->z + 1;
	}
}

void System_Border(ecs_iter_t *it)
{
	EgZIndex *z = ecs_field(it, EgZIndex, 1);
	EgZIndex *z0 = ecs_field(it, EgZIndex, 2);
	ecs_entity_t e_z0 = ecs_field_src(it, 2);
	if(z0 == NULL){return;}
	for (int i = 0; i < it->count; i ++)
	{
		//printf("%s:%i, %s:%i\n", ecs_get_name(it->world, e_z0), z0->z, ecs_get_name(it->world, it->entities[i]), z[i].z);
		z[i].z = z0->z + 1;
	}
}


void System_Fllow(ecs_iter_t *it)
{
	EgV2F32 *p = ecs_field(it, EgV2F32, 1);
	EgV2F32 *m0 = ecs_field(it, EgV2F32, 2);
	EgV2F32 *d0 = ecs_field(it, EgV2F32, 3);
	for (int i = 0; i < it->count; i ++)
	{
		p[i].x = m0->x - d0->x;
		p[i].y = m0->y - d0->y;
	}
}


void EgGuiImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgGui);
	ecs_set_name_prefix(world, "Eg");
	ECS_IMPORT(world, EgQuantities);
	ECS_IMPORT(world, EgUserinput);

	ECS_TAG_DEFINE(world, EgGuiMouseOver);
	ECS_TAG_DEFINE(world, EgGuiMouseOver1);
	ECS_TAG_DEFINE(world, EgGuiDragging);
	ECS_COMPONENT_DEFINE(world, EgMargin4);
	ECS_COMPONENT_DEFINE(world, EgZIndex);
	ECS_COMPONENT_DEFINE(world, EgHover);
	ECS_COMPONENT_DEFINE(world, EgGuiDrag);
	ECS_COMPONENT_DEFINE(world, EgGuiBorder4);

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
	.entity = ecs_id(EgGuiBorder4),
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
	{ .name = "zindex", .type = ecs_id(ecs_u32_t) },
	{ .name = "entity1", .type = ecs_id(ecs_entity_t) }
	}
	});


	ecs_struct(world, {
	.entity = ecs_id(EgGuiDrag),
	.members = {
	{ .name = "entity", .type = ecs_id(ecs_entity_t) }
	}
	});



    ecs_entity_t e_System_Hover1 = ecs_system(world, {
        .entity = ecs_entity(world, {
			.name = "System_Hover1",
			.add = { ecs_dependson(EcsOnUpdate) }
		}),
        .query.filter.terms = {
            { .id = ecs_id(EgZIndex),               .inout = EcsIn },
            { .id = ecs_id(EgPositionGlobal_V2F32), .inout = EcsIn },
            { .id = ecs_id(EgRectangle_V2F32),      .inout = EcsIn },
            { .id = ecs_id(EgPosition_V2F32), .src.id = ecs_id(EgMouse) },
            { .id = ecs_id(EgHover),          .src.id = ecs_id(EgHover) },
            { .id = ecs_id(EgPosition_V2F32), .src.id = ecs_id(EgHover) },
            { .id = ecs_id(EgGuiDragging), .oper = EcsNot },
        },
        .callback = System_Hover1
    });

    ecs_entity_t e_System_Drag1 = ecs_system(world, {
        .entity = ecs_entity(world, {
			.name = "System_Drag1",
			.add = { ecs_dependson(EcsOnUpdate) }
		}),
        .query.filter.terms = {
            { .id = ecs_id(EgPosition_V2F32),       .src.id = ecs_id(EgMouse) },
            { .id = ecs_id(EgMouse),                .src.id = ecs_id(EgMouse) },
            { .id = ecs_id(EgHover),                .src.id = ecs_id(EgHover) },
            { .id = ecs_id(EgPosition_V2F32),       .src.id = ecs_id(EgHover) },
            { .id = ecs_id(EgGuiDrag),              .src.id = ecs_id(EgGuiDrag) },
            { .id = ecs_id(EgPosition_V2F32),       .src.id = ecs_id(EgGuiDrag) },
        },
        .callback = System_Drag1
    });


    ecs_entity_t e_System_Margin = ecs_system(world, {
        .entity = ecs_entity(world, {
			.name = "System_Margin",
			.add = { ecs_dependson(EcsOnUpdate) }
		}),
        .query.filter.terms = {
            {.id = ecs_id(EgPosition_V2F32), .inout = EcsOut },
            {.id = ecs_id(EgRectangle_V2F32), .inout = EcsOut },
            {.id = ecs_id(EgMargin4), .inout = EcsIn },
            {.id = ecs_id(EgRectangle_V2F32), .inout = EcsIn, .src.flags = EcsParent, .oper = EcsOptional}
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


	ecs_system(world, {
	.entity = ecs_entity(world, {
	.name = "System_Fllow",
	.add = { ecs_dependson(EcsOnUpdate) }
	}),
	.query.filter.terms = {
	{ .id = ecs_id(EgPositionGlobal_V2F32), },
	{ .id = ecs_id(EgPosition_V2F32), .src.id = ecs_id(EgMouse) },
    { .id = ecs_id(EgPosition_V2F32), .src.id = ecs_id(EgGuiDrag) },
	{ .id = ecs_id(EgGuiDragging),},
	},
	.callback = System_Fllow
	});




}
