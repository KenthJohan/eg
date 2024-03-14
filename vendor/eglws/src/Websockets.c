#include "eglws/Websockets.h"
#include "eg/EgStr.h"
#include "ws.h"



static int banana = 1;

static void Sys1(ecs_iter_t *it)
{
	ews_t * ews = it->ctx;
	for (int i = 0; i < it->count; ++i) {
		if(ews->internal_vhd) {
			//printf("ews_send_string\n");
			char buf[128];
			snprintf(buf, sizeof(buf), "Banana %i", banana++);
			//ews_send_string(ews, buf);
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
	ECS_IMPORT(world, EgStr);

	//signal(SIGINT, sigint_handler);

	ecs_entity_t * a = ecs_new(world, 0);
	ecs_set(world, a, EgText, {"Hello"});


	ews_t * ews = ews_init();
	ews->world = world;

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "Sys1", .add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = Sys1,
	.ctx = ews,
	.query.filter.terms = {
		{.id = ecs_id(EgText)}
	}});


}