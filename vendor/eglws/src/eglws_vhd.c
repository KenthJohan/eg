#include "eglws_vhd.h"

#include "spam.h"
#include "msg.h"

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

	const eglws_msg_t *pmsg = lws_ring_get_element(vhd->ring, &pss->tail);
	if (!pmsg) {
		pthread_mutex_unlock(&vhd->lock_ring);
		return 0;
	}

	printf("lws_write: %li\n", pmsg->len);
	printf("lws_ring_get_count_waiting_elements %li\n", lws_ring_get_count_waiting_elements(vhd->ring, &pss->tail));
	
	// notice we allowed for LWS_PRE in the payload already
	int m = lws_write(wsi, ((unsigned char *)pmsg->payload) + LWS_PRE, pmsg->len, LWS_WRITE_TEXT);
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




int eglws_vhd_send_message(eglws_vhd_t * vhd, void const * data, int len)
{
	int n;
	if (!vhd->pss_list) {
		return -1;
	}
	pthread_mutex_lock(&vhd->lock_ring);
	n = (int)lws_ring_get_count_free_elements(vhd->ring);
	if (n == 0) {
		lwsl_user("dropping!\n");
		goto unlock;
	}
	eglws_msg_t msg = {0};
	eglws_msg_init(&msg, data, len);
	if (msg.payload == NULL) {
		lwsl_user("OOM: dropping\n");
		goto unlock;
	}
	n = (int)lws_ring_insert(vhd->ring, &msg, 1);
	if (n != 1) {
		eglws_msg_fini(&msg);
		lwsl_user("dropping!\n");
	} else {
		lws_cancel_service(vhd->context);
	}
unlock:
	pthread_mutex_unlock(&vhd->lock_ring);
	return 0;
}


int eglws_vhd_send_text(eglws_vhd_t * vhd, char const * text)
{
	return eglws_vhd_send_message(vhd, text, strlen(text));
}