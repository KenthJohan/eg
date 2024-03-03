#include "egwebsockets/Websockets.h"
#include "eg/Components.h"
#include "ws.h"




static void Sys1(ecs_iter_t *it)
{
	ews_t * ews = it->ctx;
	for (int i = 0; i < it->count; ++i) {
		if(ews->internal_vhd) {
			printf("ews_send_message\n");
			ews_send_message(ews, "Hej!");
		}
		else {
			printf("no vhd\n");
		}
	}
}

/*
void sigint_handler(int sig)
{
	interrupted = 1;
}
*/


void WebsocketsImport(ecs_world_t *world)
{
	ECS_MODULE(world, Websockets);
	ECS_IMPORT(world, Components);

	//signal(SIGINT, sigint_handler);

	ecs_entity_t * a = ecs_new(world, 0);
	ecs_set(world, a, String, {"Hello"});


	ews_t * ews = ews_init();
	ews->world = world;

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "Sys1", .add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = Sys1,
	.ctx = ews,
	.query.filter.terms = {
		{.id = ecs_id(String)}
	}});


}