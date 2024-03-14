#pragma once

#include <libwebsockets.h>


/* one of these created for each message in the ringbuffer */
typedef struct {
	void *payload; /* is malloc'd */
	size_t len;
	enum lws_write_protocol protocol;
} eglws_msg_t;


void eglws_msg_fini(void * ptr);

int eglws_msg_write(eglws_msg_t const *msg, struct lws *wsi);

int eglws_msg_add(eglws_msg_t * msg, struct lws_ring *ring, pthread_mutex_t *mtx);