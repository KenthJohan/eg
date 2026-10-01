#include "EgBase.h"

ECS_COMPONENT_DECLARE(EgBaseText);
ECS_COMPONENT_DECLARE(EgBaseFont);
ECS_COMPONENT_DECLARE(EgBaseColor);

ECS_CTOR(EgBaseText, ptr, {
	ptr->value = NULL;
})

ECS_MOVE(EgBaseText, dst, src, {
	ecs_os_free(dst->value);
	dst->value = src->value;
	src->value = NULL;
})

ECS_COPY(EgBaseText, dst, src, {
	ecs_os_free(dst->value);
	dst->value = src->value ? ecs_os_strdup(src->value) : NULL;
})

ECS_DTOR(EgBaseText, ptr, {
	ecs_os_free(ptr->value);
	ptr->value = NULL;
})

void EgBaseImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgBase);
	ecs_set_name_prefix(world, "EgBase");

	ECS_COMPONENT_DEFINE(world, EgBaseText);
	ECS_COMPONENT_DEFINE(world, EgBaseFont);
	ECS_COMPONENT_DEFINE(world, EgBaseColor);

	ecs_set_hooks(world, EgBaseText,
	{
	.ctor = ecs_ctor(EgBaseText),
	.move = ecs_move(EgBaseText),
	.copy = ecs_copy(EgBaseText),
	.dtor = ecs_dtor(EgBaseText),
	});

	ecs_struct(world,
	{.entity = ecs_id(EgBaseText),
	.members = {
	{.name = "value", .type = ecs_id(ecs_string_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgBaseFont),
	.members = {
	{.name = "font_size", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_struct(world,
	{.entity = ecs_id(EgBaseColor),
	.members = {
	{.name = "color", .type = ecs_id(ecs_u32_t)},
	}});
}
