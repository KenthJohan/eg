#pragma once
#include <flecs.h>

#define EWS_MAX_CHANNEL 128

typedef struct {
	int active;
	int32_t channel;
} sub_t;

typedef struct {
	ecs_world_t * world;
	void * internal_vhd;
	int should_quit;
	ecs_os_thread_t thread;
	int rc;
	sub_t subs[EWS_MAX_CHANNEL];
} ews_t;



int ews_send_binary(ews_t * ews, void const * data, int len);
int ews_send_string(ews_t * ews, char const * msg);
int ews_progress(ews_t * ews);

ews_t * ews_init();
void ews_fini(ews_t * ews);