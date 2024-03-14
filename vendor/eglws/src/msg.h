#pragma once

#include <libwebsockets.h>


/* one of these created for each message in the ringbuffer */
typedef struct {
	void *payload; /* is malloc'd */
	size_t len;
	enum lws_write_protocol protocol;
} eglws_msg_t;

void eglws_msg_init(eglws_msg_t * msg, void const * data, int len);

void eglws_msg_fini(void * ptr);

int eglws_msg_write(struct lws *wsi, eglws_msg_t const *msg);