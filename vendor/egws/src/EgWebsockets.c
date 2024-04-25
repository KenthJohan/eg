#include "egws.h"
#include "egcomponents.h"
#include "egfs.h"
#include "egstr.h"
#include "ws.h"




static void Sys1(ecs_iter_t *it)
{
	//ecs_http_server_t *srv = it->ctx;
	//ecs_http_server_dequeue(srv, it->delta_time);
	for (int i = 0; i < it->count; ++i) {

	}
}


void onopen(ws_cli_conn_t *client)
{
	((void)client);
	printf("Connected!\n");
}


void onclose(ws_cli_conn_t *client)
{
	((void)client);
	printf("Disconnected!\n");
}

static int counter = 0;
void onmessage(ws_cli_conn_t *client, const unsigned char *msg, uint64_t size, int type)
{
	((void)client);
	((void)msg);
	((void)size);
	((void)type);
	printf("onmessage: %s\n", msg);
	if (strcmp(msg, "Dummy") == 0) {
		char buf[128];
		snprintf(buf, 128, "Dummy response %i", counter++);
		ws_sendframe_txt(client, buf);
	}
}



void * ping_thread(void * arg) {
	/*
	 * Periodically send ping frames in the main thread
	 * and aborts inactive connections.
	 */
	while (1)
	{
		/*
		 * Sends a broadcast PING with 2-DELAY MS of tolerance, i.e:
		 * the client can miss up to 2 PINGs messages.
		 *
		 * The 'timeout' is specified by the time between ws_ping()
		 * calls. In this example, 10 seconds.
		 */
		printf("Sending ping...\n");
		ws_ping(NULL, 2);

		/* Sleep 10 seconds. */
		ecs_os_sleep(10, 0);
	}
	return NULL;
}







void EgWebsocketsImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgWebsockets);
	ECS_IMPORT(world, EgComponents);
	ECS_IMPORT(world, EgFs);
	ECS_IMPORT(world, EgStr);

	
	ws_socket(&(struct ws_server){
		.host = "0.0.0.0",
		.port = 8080,
		.thread_loop   = 1,
		.timeout_ms    = 1000,
		.evs.onopen    = &onopen,
		.evs.onclose   = &onclose,
		.evs.onmessage = &onmessage
	});
	ecs_os_thread_new(ping_thread, NULL);
	









	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "Sys1", .add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = Sys1,
	.ctx = NULL,
	.query.filter.terms = {
		{.id = EgFsLoad}
	}});



}