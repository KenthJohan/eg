#pragma once


#include <libwebsockets.h>

/* one of these created for each message in the ringbuffer */

struct msg {
	void *payload; /* is malloc'd */
	size_t len;
};

/*
 * One of these is created for each client connecting to us.
 *
 * It is ONLY read or written from the lws service thread context.
 */

struct per_session_data__minimal {
	struct per_session_data__minimal *pss_list;
	struct lws *wsi;
	uint32_t tail;
};

/* one of these is created for each vhost our protocol is used with */

struct per_vhost_data__minimal {
	struct lws_context *context;
	struct lws_vhost *vhost;
	const struct lws_protocols *protocol;
	struct per_session_data__minimal *pss_list; /* linked-list of live pss*/
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
};

/*
 * This runs under both lws service and "spam threads" contexts.
 * Access is serialized by vhd->lock_ring.
 */

static void __minimal_destroy_message(void *_msg)
{
	struct msg *msg = _msg;
	free(msg->payload);
	msg->payload = NULL;
	msg->len = 0;
}