#include "egwebsockets/Websockets.h"
#include "eg/Components.h"
#include "ws.h"




static void Sys1(ecs_iter_t *it)
{
	for (int i = 0; i < it->count; ++i) {
		send_message("Hej!");
	}
}



void WebsocketsImport(ecs_world_t *world)
{
	ECS_MODULE(world, Websockets);
	ECS_IMPORT(world, Components);

	ecs_os_thread_t t = ecs_os_thread_new(server_thread, NULL);

	ecs_entity_t * a = ecs_new(world, 0);
	ecs_set(world, a, String, {"Hello"});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "Sys1", .add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = Sys1,
	.query.filter.terms = {
		{.id = ecs_id(String)}
	}});


}