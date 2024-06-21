#include "egstr.h"




#define FLOG(...) fprintf(__VA_ARGS__)


ECS_COMPONENT_DECLARE(EgStrText);
ECS_COMPONENT_DECLARE(EgStrBuffer);



ECS_COPY(EgStrText, dst, src, {
	FLOG(stdout, "EgText::Copy\n");
	ecs_os_strset((char**)&dst->value, src->value);
})

ECS_MOVE(EgStrText, dst, src, {
	FLOG(stdout, "EgText::MOVE\n");
	ecs_os_free((char*)dst->value);
	dst->value = src->value;
	src->value = NULL;
})

ECS_DTOR(EgStrText, ptr, {
	FLOG(stdout, "EgText::DTOR\n");
	ecs_os_free((char*)ptr->value);
})

ECS_CTOR(EgStrBuffer, ptr, {
	FLOG(stdout, "EgBuffer::Ctor\n");
	ptr->data = NULL;
})

ECS_DTOR(EgStrBuffer, ptr, {
	FLOG(stdout, "EgBuffer::Dtor %p\n", ptr->data);
	if(ptr->data){ecs_os_free(ptr->data);}
})

ECS_MOVE(EgStrBuffer, dst, src, {
	FLOG(stdout, "EgBuffer::Move %p %p\n", dst->data, src->data);
	ecs_os_free((char*)dst->data);
	dst->data = src->data;
	dst->size = src->size;
	src->data = NULL;
})

ECS_COPY(EgStrBuffer, dst, src, {
	FLOG(stdout, "EgBuffer::Copy %p %p\n", dst->data, src->data);
	dst->data = ecs_os_memdup(src->data, src->size);
	dst->size = src->size;
})



void EgStrImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgStr);
	ecs_set_name_prefix(world, "EgStr");

	ECS_COMPONENT_DEFINE(world, EgStrText);
	ECS_COMPONENT_DEFINE(world, EgStrBuffer);


	ecs_set_hooks(world, EgStrText, {
	.ctor = ecs_default_ctor,
	.move = ecs_move(EgStrText),
	.copy = ecs_copy(EgStrText),
	.dtor = ecs_dtor(EgStrText)
	});

	ecs_set_hooks(world, EgStrBuffer, {
	.ctor = ecs_default_ctor,
	.move = ecs_move(EgStrBuffer),
	.copy = ecs_copy(EgStrBuffer),
	.dtor = ecs_dtor(EgStrBuffer)
	});
 
	ecs_struct(world, {
	.entity = ecs_id(EgStrText),
	.members = {
	{ .name = "value", .type = ecs_id(ecs_string_t) },
	}
	});

	ecs_struct(world, {
	.entity = ecs_id(EgStrBuffer),
	.members = {
	{ .name = "size", .type = ecs_id(ecs_i32_t) },
	{ .name = "data", .type = ecs_id(ecs_uptr_t) }
	}
	});

}