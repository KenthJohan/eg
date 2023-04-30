#include "EgQuantities.h"
#include "eg_basics.h"
#include <math.h>
#include <stdio.h>



#define FLOG(...) fprintf(__VA_ARGS__)

/*
ECS_COMPONENT_DECLARE(EgV1F32);
ECS_COMPONENT_DECLARE(EgV2F32);
ECS_COMPONENT_DECLARE(EgV3F32);
ECS_COMPONENT_DECLARE(EgV4F32);
ECS_COMPONENT_DECLARE(EgV4U8);
*/
ECS_COMPONENT_DECLARE(EgVelocity_V2F32);
ECS_COMPONENT_DECLARE(EgVelocity_V3F32);
ECS_COMPONENT_DECLARE(EgPosition_V2F32);
ECS_COMPONENT_DECLARE(EgPosition_V3F32);
ECS_COMPONENT_DECLARE(EgPositionGlobal_V2F32);
ECS_COMPONENT_DECLARE(EgPositionGlobal_V3F32);
ECS_COMPONENT_DECLARE(EgRectangle_V2F32);
ECS_COMPONENT_DECLARE(EgColorRGBA_V4U8);
ECS_COMPONENT_DECLARE(EgMassF32);


ECS_COMPONENT_DECLARE(EgText);
ECS_COMPONENT_DECLARE(EgBuffer);





ECS_COPY(EgText, dst, src, {
ecs_os_strset((char**)&dst->value, src->value);
})

ECS_MOVE(EgText, dst, src, {
ecs_os_free((char*)dst->value);
dst->value = src->value;
src->value = NULL;
})

ECS_DTOR(EgText, ptr, {
ecs_os_free((char*)ptr->value);
})




ECS_CTOR(EgBuffer, ptr, {
	FLOG(stdout, "EgBuffer::Ctor\n");
	ptr->data = NULL;
})


ECS_DTOR(EgBuffer, ptr, {
	FLOG(stdout, "EgBuffer::Dtor\n");
	if(ptr->data){ecs_os_free(ptr->data);}
})

// The move hook should move resources from one location to another.
ECS_MOVE(EgBuffer, dst, src, {
	FLOG(stdout, "EgBuffer::Move\n");
	ecs_os_free((char*)dst->data);
	dst->data = src->data;
	src->data = NULL;
})

// The copy hook should copy resources from one location to another.
ECS_COPY(EgBuffer, dst, src, {
	FLOG(stdout, "EgBuffer::Copy\n");
})







void Move(ecs_iter_t *it)
{
	const EgPosition_V2F32       *r  = ecs_field(it, EgPosition_V2F32, 1);
	      EgPositionGlobal_V2F32 *p  = ecs_field(it, EgPositionGlobal_V2F32, 2);
	const EgPositionGlobal_V2F32 *p0 = ecs_field(it, EgPositionGlobal_V2F32, 3);
	if(p0)
	{
		for (int i = 0; i < it->count; i ++)
		{
			p[i].x = p0->x + r[i].x;
			p[i].y = p0->y + r[i].y;
			//printf("%s: {%f, %f}\n", ecs_get_name(it->world, it->entities[i]), p[i].x, p[i].y);
			//printf("%s:\n", ecs_get_name(it->world, it->entities[i]));
		}
	}
	else
	{
		for (int i = 0; i < it->count; i ++)
		{
			p[i].x = r[i].x;
			p[i].y = r[i].y;
		}
	}
}



void Random_Color(ecs_iter_t *it)
{
	EgColorRGBA_V4U8 *c = ecs_field(it, EgColorRGBA_V4U8, 1);
	for (int i = 0; i < it->count; i ++)
	{
		c[i].r = (uint8_t)rand();
		c[i].g = (uint8_t)rand();
		c[i].b = (uint8_t)rand();
		c[i].a = 255;
	}
}





void EgQuantitiesImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgQuantities);
	ecs_set_name_prefix(world, "Eg");
	
	/*
	ECS_COMPONENT_DEFINE(world, EgV1F32);
	ECS_COMPONENT_DEFINE(world, EgV2F32);
	ECS_COMPONENT_DEFINE(world, EgV3F32);
	ECS_COMPONENT_DEFINE(world, EgV4F32);
	ECS_COMPONENT_DEFINE(world, EgV4U8);
	*/
	ECS_COMPONENT_DEFINE(world, EgVelocity_V2F32);
	ECS_COMPONENT_DEFINE(world, EgVelocity_V3F32);
	ECS_COMPONENT_DEFINE(world, EgPosition_V2F32);
	ECS_COMPONENT_DEFINE(world, EgPosition_V3F32);
	ECS_COMPONENT_DEFINE(world, EgPositionGlobal_V2F32);
	ECS_COMPONENT_DEFINE(world, EgPositionGlobal_V3F32);
	ECS_COMPONENT_DEFINE(world, EgRectangle_V2F32);
	ECS_COMPONENT_DEFINE(world, EgColorRGBA_V4U8);
	ECS_COMPONENT_DEFINE(world, EgMassF32);

	ECS_COMPONENT_DEFINE(world, EgText);
	ECS_COMPONENT_DEFINE(world, EgBuffer);

	/*
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
	*/

	ecs_struct(world, {
	.entity = ecs_id(EgColorRGBA_V4U8),
	.members = {
	{ .name = "r", .type = ecs_id(ecs_u8_t) },
	{ .name = "g", .type = ecs_id(ecs_u8_t) },
	{ .name = "b", .type = ecs_id(ecs_u8_t) },
	{ .name = "a", .type = ecs_id(ecs_u8_t) }
	}
	});

	ecs_struct(world, {
	.entity = ecs_id(EgPosition_V2F32),
	.members = {
	{ .name = "x", .type = ecs_id(ecs_f32_t) },
	{ .name = "y", .type = ecs_id(ecs_f32_t) },
	}
	});

/*
    ecs_metric(world, {
        .entity = ecs_entity(world, { .name = "metrics.position_y" }),
        .member = ecs_lookup_fullpath(world, "eg.quantities.Position_V2F32.y"),
        .kind = EcsGauge
    });
	*/



	ecs_struct(world, {
	.entity = ecs_id(EgVelocity_V2F32),
	.members = {
	{ .name = "x", .type = ecs_id(ecs_f32_t) },
	{ .name = "y", .type = ecs_id(ecs_f32_t) },
	}
	});

	ecs_struct(world, {
	.entity = ecs_id(EgPositionGlobal_V2F32),
	.members = {
	{ .name = "x", .type = ecs_id(ecs_f32_t) },
	{ .name = "y", .type = ecs_id(ecs_f32_t) },
	}
	});

	ecs_struct(world, {
	.entity = ecs_id(EgRectangle_V2F32),
	.members = {
	{ .name = "w", .type = ecs_id(ecs_f32_t) },
	{ .name = "h", .type = ecs_id(ecs_f32_t) },
	}
	});

	ecs_struct(world, {
	.entity = ecs_id(EgText),
	.members = {
	{ .name = "value", .type = ecs_id(ecs_string_t) }
	}
	});

	ecs_set_hooks(world, EgText, {
	.ctor = ecs_default_ctor,
	.move = ecs_move(EgText),
	.copy = ecs_copy(EgText),
	.dtor = ecs_dtor(EgText)
	});

	ecs_set_hooks(world, EgBuffer, {
	.ctor = ecs_default_ctor,
	.move = ecs_move(EgBuffer),
	.copy = ecs_copy(EgBuffer),
	.dtor = ecs_dtor(EgBuffer)
	});
 

	ecs_system(world, {
		.entity = ecs_entity(world, {
		.name = "Move",
		.add = { ecs_dependson(EcsOnUpdate) }
		}),
		.query.filter.terms = {
			{.id = ecs_id(EgPosition_V2F32), .inout = EcsIn },
			{.id = ecs_id(EgPositionGlobal_V2F32), .inout = EcsOut },
			{.id = ecs_id(EgPositionGlobal_V2F32), .inout = EcsIn,.src.flags = EcsParent | EcsCascade,.oper = EcsOptional}
		},
		.callback = Move
	});

	ecs_observer(world, {
		.filter = { .terms = {{ .id = ecs_id(EgColorRGBA_V4U8) }}},
		.events = { EcsOnAdd },
		.callback = Random_Color
	});

}
