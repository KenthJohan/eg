#pragma once


typedef struct {
	void * internal;
} ews_t;

void * server_thread(void* arg);

void send_message(ews_t * ews, char const * msg);