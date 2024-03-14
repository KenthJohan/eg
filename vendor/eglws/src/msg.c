#include "msg.h"


/*
 * This runs under both lws service and "spam threads" contexts.
 * Access is serialized by vhd->lock_ring.
 */

void eglws_msg_fini(void * ptr)
{
	eglws_msg_t *msg = ptr;
	free(msg->payload);
	msg->payload = NULL;
	msg->len = 0;
}


int eglws_msg_write(struct lws *wsi, eglws_msg_t const *msg)
{
	int m = lws_write(wsi, ((unsigned char *)msg->payload) + LWS_PRE, msg->len, LWS_WRITE_TEXT);
	return m;
}


void eglws_msg_init(eglws_msg_t * msg, void const * data, int len)
{
	msg->payload = malloc((unsigned int)(LWS_PRE + len));
	if (msg->payload == NULL) {
		return;
	}
	memcpy((char*)msg->payload + LWS_PRE, data, len);
	msg->len = len;
}