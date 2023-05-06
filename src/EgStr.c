#include "EgStr.h"
#include "eg_basics.h"
#include <math.h>
#include <stdio.h>



#define FLOG(...) fprintf(__VA_ARGS__)


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
 

}
