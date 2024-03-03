#pragma once
#include <flecs.h>

typedef struct {
	ecs_world_t * world;
	void * internal_vhd;
	int should_quit;
} ews_t;



void ews_send_message(ews_t * ews, char const * msg);


ews_t * ews_init();
void ews_fini(ews_t * ews);