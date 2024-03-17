#include "msg.h"


/*
 * This runs under both lws service and "spam threads" contexts.
 * Access is serialized by vhd->lock_ring.
 */

void eglws_msg_fini(void * ptr)
{
	printf("eglws_msg_fini()\n");
	eglws_msg_t *msg = ptr;
	free(msg->payload);
	msg->payload = NULL;
	msg->len = 0;
}


int eglws_msg_write(eglws_msg_t const *msg, struct lws *wsi)
{
	// notice we allowed for LWS_PRE in the payload already
	int m = lws_write(wsi, ((unsigned char *)msg->payload) + LWS_PRE, msg->len, msg->protocol);
	return m;
}


int eglws_msg_add(eglws_msg_t * msg, struct lws_ring *ring, pthread_mutex_t *mtx)
{
	int n;
	pthread_mutex_lock(mtx);
	n = (int)lws_ring_get_count_free_elements(ring);
	if (n == 0) {
		pthread_mutex_unlock(mtx);
		return -1;
	}
	n = (int)lws_ring_insert(ring, msg, 1);
	if (n != 1) {
		pthread_mutex_unlock(mtx);
		return -1;
	}
	pthread_mutex_unlock(mtx);
	return 0;
}