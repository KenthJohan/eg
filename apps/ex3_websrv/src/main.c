#include <flecs.h>
#include <stdio.h>
//#include <egws/Websockets.h>
#include <eg/EgWebservers.h>
#include <eg/EgStr.h>
#include <eg/EgFs.h>



typedef struct {
	int dummy;
} Comp1;

typedef struct {
	int dummy;
} Comp2;

ECS_COMPONENT_DECLARE(Comp1);
ECS_COMPONENT_DECLARE(Comp2);



void Sys1(ecs_iter_t *it)
{
	Comp1 *points = ecs_field(it, Comp1, 1);
	for (int i = 0; i < it->count; ++i) {
		//printf("Sys1\n");
	}
}

void Sys2(ecs_iter_t *it)
{
	Comp2 *points = ecs_field(it, Comp2, 1);
	for (int i = 0; i < it->count; ++i) {
		//printf("Sys2\n");
	}
}






int main(int argc, char const * argv[])
{
	ecs_world_t *world = ecs_init();
	//ECS_IMPORT(world, Websockets);
	ECS_IMPORT(world, EgWebservers);
	ECS_IMPORT(world, EgFs);
	ECS_IMPORT(world, EgStr);
	
	// https://www.flecs.dev/explorer/?remote=true
	ecs_set(world, EcsWorld, EcsRest, {.port = 0});

	{
		ecs_entity_t root = ecs_new_entity(world, "webroot");
		ecs_set_pair(world, root, EgText, EgFsRoot, {"./website"});
		EgHttp_add_file(world, root, "webroot/index.html");
		EgHttp_add_file(world, root, "webroot/example.js");
		EgHttp_add_file(world, root, "webroot/favicon.ico");
		EgHttp_add_file(world, root, "webroot/sub1/index.html");
		EgHttp_add_file(world, root, "webroot/sub1/script.js");
		EgHttp_add_file(world, root, "webroot/sub2/index.html");
		EgHttp_add_file(world, root, "webroot/sub2/example.js");
		EgHttp_add_file(world, root, "webroot/webdesign.css");
		ecs_entity_t e = ecs_new_entity(world, "websrv1");
		ecs_set(world, e, EgWebsrv, {.root = root});
	}



	ECS_COMPONENT_DEFINE(world, Comp1);
	ECS_COMPONENT_DEFINE(world, Comp2);

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = Sys1,
	.multi_threaded = true,
	.query.filter.terms =
	{
	{.id = ecs_id(Comp1), .src.flags = EcsSelf},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = Sys2,
	.multi_threaded = true,
	.query.filter.terms =
	{
	{.id = ecs_id(Comp2), .src.flags = EcsSelf},
	}});

	ecs_entity_t e1 = ecs_new(world, Comp1);
	ecs_entity_t e2 = ecs_new(world, Comp2);

	while(1) {
		ecs_os_sleep(0.0f, 100000.0f);
		//printf("ecs_progress\n");
		ecs_progress(world, 0.0f);
	}

}
