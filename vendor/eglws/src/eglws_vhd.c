#include "eglws_vhd.h"

#include "spam.h"
#include "msg.h"


/*
https://github.com/warmcat/libwebsockets/blob/main/minimal-examples-lowlevel/ws-server/minimal-ws-server-ring/protocol_lws_minimal.c

*/


eglws_vhd_t * eglws_vhd_init(struct lws *wsi, void *in)
{
	// create our per-vhost struct
	eglws_vhd_t * vhd = lws_protocol_vh_priv_zalloc(lws_get_vhost(wsi), lws_get_protocol(wsi), sizeof(eglws_vhd_t));
	if (!vhd) {
		return NULL;
	}
	pthread_mutex_init(&vhd->lock_ring, NULL);
	{
		// recover the pointer to the globals struct
		const struct lws_protocol_vhost_options *pvo;
		pvo = lws_pvo_search((const struct lws_protocol_vhost_options *)in, "config");
		if (!pvo || !pvo->value) {
			lwsl_err("%s: Can't find \"config\" pvo\n", __func__);
			return NULL;
		}
		vhd->config = pvo->value;
	}
	vhd->context = lws_get_context(wsi);
	vhd->protocol = lws_get_protocol(wsi);
	vhd->vhost = lws_get_vhost(wsi);
	vhd->ring = lws_ring_create(sizeof(eglws_msg_t), 8, eglws_msg_fini);
	if (!vhd->ring) {
		lwsl_err("%s: failed to create ring\n", __func__);
		return NULL;
	}

	// start the content-creating threads
	//spam_start(vhd);
	return vhd;
}


void eglws_vhd_fini(eglws_vhd_t * vhd)
{
	spam_fini(vhd);
	if (vhd->ring) {
		lws_ring_destroy(vhd->ring);
	}
	pthread_mutex_destroy(&vhd->lock_ring);
}


int eglws_vhd_consume(eglws_vhd_t * vhd, eglws_pss_t * pss, struct lws *wsi)
{
	pthread_mutex_lock(&vhd->lock_ring);

	/*
	lws_start_foreach_llp(eglws_pss_t **, ppss, vhd->pss_list) {
		int fd1 = lws_get_socket_fd(wsi);
		int fd2 = lws_get_socket_fd((*ppss)->wsi);
		printf("consume %i %i\n", fd1, fd2);
	} lws_end_foreach_llp(ppss, pss_list);
	*/

	int n = lws_ring_get_count_waiting_elements(vhd->ring, NULL);
	printf("lws_ring_get_count_waiting_elementsA %i\n", n);

	const eglws_msg_t *pmsg = lws_ring_get_element(vhd->ring, &pss->tail);
	if (!pmsg) {
		pthread_mutex_unlock(&vhd->lock_ring);
		return 0;
	}


	//printf("lws_write: %li\n", pmsg->len);
	//printf("lws_ring_get_count_waiting_elements %li\n", lws_ring_get_count_waiting_elements(vhd->ring, &pss->tail));


	int m = eglws_msg_write(pmsg, wsi);
	if (m < (int)pmsg->len) {
		pthread_mutex_unlock(&vhd->lock_ring);
		lwsl_err("ERROR %d writing to ws socket\n", m);
		return -1;
	}

	// This will call the destroy callback specified in lws_ring_create()
	lws_ring_consume_and_update_oldest_tail(
		vhd->ring,	/* lws_ring object */
		eglws_pss_t, /* type of objects with tails */
		&pss->tail,	/* tail of guy doing the consuming */
		1,		/* number of payload objects being consumed */
		vhd->pss_list,	/* head of list of objects with tails */
		tail,		/* member name of tail in objects with tails */
		pss_list	/* member name of next object in objects with tails */
	);

	// more to do?
	if (lws_ring_get_element(vhd->ring, &pss->tail)) {
		// come back as soon as we can write more
		lws_callback_on_writable(pss->wsi);
	}

	n = lws_ring_get_count_waiting_elements(vhd->ring, NULL);
	printf("lws_ring_get_count_waiting_elementsB %i\n", n);


	pthread_mutex_unlock(&vhd->lock_ring);
	return 0;
}


int eglws_vhd_request_writable(eglws_vhd_t * vhd)
{
	/*
		* When the "spam" threads add a message to the ringbuffer,
		* they create this event in the lws service thread context
		* using lws_cancel_service().
		*
		* We respond by scheduling a writable callback for all
		* connected clients.
		*/
	lws_start_foreach_llp(eglws_pss_t **, ppss, vhd->pss_list) {
		lws_callback_on_writable((*ppss)->wsi);
	} lws_end_foreach_llp(ppss, pss_list);
	return 0;
}




int eglws_vhd_send_binary(struct lws_ring *ring, pthread_mutex_t *mtx, void const * data, int len)
{
	/*
	if (!vhd->pss_list) {
		return -1;
	}
	*/
	eglws_msg_t msg = {0};
	msg.protocol = LWS_WRITE_BINARY;
	msg.payload = malloc((unsigned int)(LWS_PRE + len));
	memcpy((char*)msg.payload + LWS_PRE, data, len);
	msg.len = len;
	int rc;
	rc = eglws_msg_add(&msg, ring, mtx);
	if (rc) {
		free(msg.payload);
	}
	//lws_cancel_service(vhd->context);
	return rc;
}

int eglws_vhd_send_text(struct lws_ring *ring, pthread_mutex_t *mtx, char const * text)
{
	/*
	if (!vhd->pss_list) {
		return -1;
	}
	*/
	eglws_msg_t msg = {0};
	msg.protocol = LWS_WRITE_TEXT;
	int len = strlen(text);
	msg.payload = malloc((unsigned int)(LWS_PRE + len));
	memcpy((char*)msg.payload + LWS_PRE, text, len);
	msg.len = len;
	int rc;
	rc = eglws_msg_add(&msg, ring, mtx);
	if (rc) {
		free(msg.payload);
	}
	//lws_cancel_service(vhd->context);
	return rc;
}





