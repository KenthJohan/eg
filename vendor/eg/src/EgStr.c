#include "eg/EgStr.h"




#define FLOG(...) fprintf(__VA_ARGS__)


ECS_COMPONENT_DECLARE(EgText);
ECS_COMPONENT_DECLARE(EgBuffer);



ECS_COPY(EgText, dst, src, {
	FLOG(stdout, "EgText::Copy\n");
	ecs_os_strset((char**)&dst->value, src->value);
})

ECS_MOVE(EgText, dst, src, {
	FLOG(stdout, "EgText::MOVE\n");
	ecs_os_free((char*)dst->value);
	dst->value = src->value;
	src->value = NULL;
})

ECS_DTOR(EgText, ptr, {
	FLOG(stdout, "EgText::DTOR\n");
	ecs_os_free((char*)ptr->value);
})

ECS_CTOR(EgBuffer, ptr, {
	FLOG(stdout, "EgBuffer::Ctor\n");
	ptr->data = NULL;
})

ECS_DTOR(EgBuffer, ptr, {
	FLOG(stdout, "EgBuffer::Dtor %p\n", ptr->data);
	if(ptr->data){ecs_os_free(ptr->data);}
})

ECS_MOVE(EgBuffer, dst, src, {
	FLOG(stdout, "EgBuffer::Move %p %p\n", dst->data, src->data);
	ecs_os_free((char*)dst->data);
	dst->data = src->data;
	dst->size = src->size;
	src->data = NULL;
})

ECS_COPY(EgBuffer, dst, src, {
	FLOG(stdout, "EgBuffer::Copy %p %p\n", dst->data, src->data);
	dst->data = ecs_os_memdup(src->data, src->size);
	dst->size = src->size;
})



void EgStrImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgStr);
	ecs_set_name_prefix(world, "Eg");

	ECS_COMPONENT_DEFINE(world, EgText);
	ECS_COMPONENT_DEFINE(world, EgBuffer);


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
 
	ecs_struct(world, {
	.entity = ecs_id(EgText),
	.members = {
	{ .name = "value", .type = ecs_id(ecs_string_t) },
	}
	});

	ecs_struct(world, {
	.entity = ecs_id(EgBuffer),
	.members = {
	{ .name = "size", .type = ecs_id(ecs_i32_t) },
	{ .name = "data", .type = ecs_id(ecs_uptr_t) }
	}
	});

}