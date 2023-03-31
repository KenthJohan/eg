#include "EgQuantities.h"
#include "eg_basics.h"
#include <math.h>


ECS_DECLARE(EgPosition);
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




void EgQuantitiesImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgQuantities);
	ecs_set_name_prefix(world, "Eg");


	ECS_TAG_DEFINE(world, EgPosition);
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


}
