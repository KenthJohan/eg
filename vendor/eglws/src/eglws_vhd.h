#pragma once

#include <libwebsockets.h>



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




eglws_vhd_t * eglws_vhd_init(struct lws *wsi, void *in);

void eglws_vhd_fini(eglws_vhd_t * vhd);

int eglws_vhd_consume(eglws_vhd_t * vhd, eglws_pss_t * pss, struct lws *wsi);

int eglws_vhd_request_writable(eglws_vhd_t * vhd);

int eglws_vhd_send_binary(struct lws_ring *ring, pthread_mutex_t *mtx, void const * data, int len);

int eglws_vhd_send_text(struct lws_ring *ring, pthread_mutex_t *mtx, char const * text);