#pragma once


#include <libwebsockets.h>

/* one of these created for each message in the ringbuffer */

typedef struct {
	void *payload; /* is malloc'd */
	size_t len;
} eglws_msg_t;

/*
 * One of these is created for each client connecting to us.
 *
 * It is ONLY read or written from the lws service thread context.
 */


typedef struct eglws_pss_t {
	struct eglws_pss_t *pss_list;
	struct lws *wsi;
	uint32_t tail;
} eglws_pss_t;

/* one of these is created for each vhost our protocol is used with */

typedef struct {
	struct lws_context *context;
	struct lws_vhost *vhost;
	const struct lws_protocols *protocol;
	eglws_pss_t *pss_list; /* linked-list of live pss*/
	const char *config;

	// Serialize access to the ring buffer
	// Destroyed on LWS_CALLBACK_PROTOCOL_DESTROY
	pthread_mutex_t lock_ring; 

	// {lock_ring} ringbuffer holding unsent content
	// Destroyed on LWS_CALLBACK_PROTOCOL_DESTROY
	struct lws_ring *ring;


	// Optional spam thread
	pthread_t spam_pthread[1];
	char spam_finished;
} eglws_vhd_t;

/*
 * This runs under both lws service and "spam threads" contexts.
 * Access is serialized by vhd->lock_ring.
 */

static void eglws_msg_fini(void * ptr)
{
	eglws_msg_t *msg = ptr;
	free(msg->payload);
	msg->payload = NULL;
	msg->len = 0;
}