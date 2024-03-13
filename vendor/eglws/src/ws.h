#pragma once
#include <flecs.h>

typedef struct {
	ecs_world_t * world;
	void * internal_vhd;
	int should_quit;
	ecs_os_thread_t thread;
} ews_t;



int ews_send_message(ews_t * ews, void const * data, int len);
int ews_send_string(ews_t * ews, char const * msg);


ews_t * ews_init();
void ews_fini(ews_t * ews);