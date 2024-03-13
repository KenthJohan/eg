#include "ws.h"
#include "eglws/Websockets.h"
#include "eg/Components.h"
#include <assert.h>

#include <libwebsockets.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>

#include "lws_misc.h"
#include "wstypes.h"
#include "spam.h"





static int _send_message(eglws_vhd_t * vhd, void const * data, int len)
{
	eglws_msg_t msg;

	if (!vhd->pss_list) {
		return -1;
	}

	pthread_mutex_lock(&vhd->lock_ring);

	{
		int n = (int)lws_ring_get_count_free_elements(vhd->ring);
		if (n == 0) {
			lwsl_user("dropping!\n");
			goto unlock;
		}
	}

	msg.payload = malloc((unsigned int)(LWS_PRE + len));
	if (msg.payload == NULL) {
		lwsl_user("OOM: dropping\n");
		goto unlock;
	}

	memcpy((char*)msg.payload + LWS_PRE, data, len);
	msg.len = len;


	{
		int n = (int)lws_ring_insert(vhd->ring, &msg, 1);
		if (n != 1) {
			eglws_msg_fini(&msg);
			lwsl_user("dropping!\n");
		} else {
			lws_cancel_service(vhd->context);
		}
	}

unlock:
	pthread_mutex_unlock(&vhd->lock_ring);
	return 0;
}







static eglws_vhd_t * eglws_vhd_init(struct lws *wsi, void *in)
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

	{

		ews_t * a = lws_vhost_user(lws_get_vhost(wsi));
		a->internal_vhd = vhd;
	}

	// start the content-creating threads
	//spam_start(vhd);
	return vhd;
}


static void eglws_vhd_fini(eglws_vhd_t * vhd)
{
	spam_fini(vhd);
	if (vhd->ring) {
		lws_ring_destroy(vhd->ring);
	}
	pthread_mutex_destroy(&vhd->lock_ring);
}


static int eglws_vhd_consume(eglws_vhd_t * vhd, eglws_pss_t * pss, struct lws *wsi)
{
	pthread_mutex_lock(&vhd->lock_ring);

	const eglws_msg_t *pmsg = lws_ring_get_element(vhd->ring, &pss->tail);
	if (!pmsg) {
		pthread_mutex_unlock(&vhd->lock_ring);
		return 0;
	}

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

	printf("lws_ring_get_count_waiting_elements %li\n", lws_ring_get_count_waiting_elements(vhd->ring, &pss->tail));

	pthread_mutex_unlock(&vhd->lock_ring);
	return 0;
}


static int eglws_vhd_broadcast(eglws_vhd_t * vhd, void * in, int len)
{
	int rc = _send_message(vhd, in, len);
	if(rc) {
		return 1;
	}
	/*
		* let everybody know we want to write something on them
		* as soon as they are ready
		*/
	lws_start_foreach_llp(eglws_pss_t **, ppss, vhd->pss_list) {
		lws_callback_on_writable((*ppss)->wsi);
	} lws_end_foreach_llp(ppss, pss_list);
}






/* this runs under the lws service thread context only */
static int _callback_minimal(struct lws *wsi, enum lws_callback_reasons reason,void *user, void *in, size_t len)
{
	eglws_pss_t *pss = (eglws_pss_t *)user;
	eglws_vhd_t *vhd = (eglws_vhd_t *)lws_protocol_vh_priv_get(lws_get_vhost(wsi), lws_get_protocol(wsi));
	
	

	{
		char ip[256];
		if (lws_get_peer_simple(wsi, ip, 16)) {
			printf("reason: %s, ip:%s\n", lws_callback_reasons_tostr(reason), ip);
		} else {
			printf("reason: %s\n", lws_callback_reasons_tostr(reason));
		}
	}

	

	switch (reason) {
	case LWS_CALLBACK_PROTOCOL_INIT:
		assert(vhd == NULL);
		vhd = eglws_vhd_init(wsi, in);
		if(vhd == NULL) {
			return 1;
		}
		break;

	case LWS_CALLBACK_PROTOCOL_DESTROY:
		eglws_vhd_fini(vhd);
		break;

	case LWS_CALLBACK_ESTABLISHED:
		/* add ourselves to the list of live pss held in the vhd */
		lws_ll_fwd_insert(pss, pss_list, vhd->pss_list);
		pss->tail = lws_ring_get_oldest_tail(vhd->ring);
		pss->wsi = wsi;
		break;

	case LWS_CALLBACK_CLOSED:
		/* remove our closing pss from the list of live pss */
		lws_ll_fwd_remove(eglws_pss_t, pss_list, pss, vhd->pss_list);
		break;

	case LWS_CALLBACK_SERVER_WRITEABLE:
		eglws_vhd_consume(vhd, pss, wsi);
		break;

	case LWS_CALLBACK_RECEIVE:
		lwsl_user("LWS_CALLBACK_RECEIVE: %4d (rpp %5d, first %d, ""last %d, bin %d, len %d)\n",(int)len, (int)lws_remaining_packet_payload(wsi),lws_is_first_fragment(wsi),lws_is_final_fragment(wsi),lws_frame_is_binary(wsi), (int)len);
		eglws_vhd_broadcast(vhd, in, len);
		break;

	case LWS_CALLBACK_EVENT_WAIT_CANCELLED:
		if (!vhd) {
			break;
		}

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
		break;

	default:
		break;
	}

	return 0;
}



static struct lws_protocols protocols[] = {
	{ 
		.name = "http", 
		.callback = lws_callback_http_dummy, 
		.per_session_data_size = 0, 
		.rx_buffer_size = 0, 
		.id = 0, 
		.user = NULL, 
		.tx_packet_size = 0 
	},
	{
		.name = "lws-minimal",
		.callback = _callback_minimal,
		.per_session_data_size = sizeof(eglws_pss_t),
		.rx_buffer_size = 128,
		.id = 0,
		.user = NULL,
		.tx_packet_size = 0
	},
	LWS_PROTOCOL_LIST_TERM
};



static const struct lws_http_mount mount = {
	.mountpoint = "/",		    
	.origin = "./mount-origin", 
	.def = "index.html",	  
	.origin_protocol = LWSMPRO_FILE,	    
	.mountpoint_len = 1,	
};

/*
 * This demonstrates how to pass a pointer into a specific protocol handler
 * running on a specific vhost.  In this case, it's our default vhost and
 * we pass the pvo named "config" with the value a const char * "myconfig".
 *
 * This is the preferred way to pass configuration into a specific vhost +
 * protocol instance.
 */

static const struct lws_protocol_vhost_options pvo_ops = {
	.next = NULL,
	.options = NULL,
	.name = "config",		/* pvo name */
	.value = (void *)"myconfig"	/* pvo value */
};

static const struct lws_protocol_vhost_options pvo = {
	.next = NULL,		/* "next" pvo linked-list */
	.options = &pvo_ops,	/* "child" pvo linked-list */
	.name = "lws-minimal",	/* protocol name we belong to on this vhost */
	.value = ""		/* ignored */
};







static void * thread_server(void* arg)
{
	ews_t * ews = (ews_t *)arg;

	int logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE
			/* for LLL_ verbosity above NOTICE to be built into lws,
			 * lws must have been configured and built with
			 * -DCMAKE_BUILD_TYPE=DEBUG instead of =RELEASE */
			/* | LLL_INFO */ /* | LLL_PARSER */ /* | LLL_HEADER */
			/* | LLL_EXT */ /* | LLL_CLIENT */ /* | LLL_LATENCY */
			/* | LLL_DEBUG */;

	

	lws_set_log_level(logs, NULL);
	lwsl_user("LWS minimal ws server + threads | visit http://localhost:7681\n");

	struct lws_context_creation_info info;
	memset(&info, 0, sizeof(info));
	info.port = 7681;
	info.mounts = &mount;
	info.protocols = protocols;
	info.pvo = &pvo; // per-vhost options
	info.options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;
	info.user = ews;

	struct lws_context *context = lws_create_context(&info);
	if (!context) {
		lwsl_err("lws init failed\n");
		return NULL;
	}

	while (ews->should_quit == 0) {
		// Service any pending websocket activity
		int rc = lws_service(context, 0);
		if (rc) {
			break;
		}
	}

	lws_context_destroy(context);

	return NULL;
}



















ews_t * ews_init() {
	ews_t * ews = ecs_os_calloc_t(ews_t);
	ews->thread = ecs_os_thread_new(thread_server, ews);
	return ews;
}


void ews_fini(ews_t * ews) {
	ecs_os_free(ews);
}

int ews_send_message(ews_t * ews, void const * data, int len)
{
	return _send_message(ews->internal_vhd, data, len);
}



int ews_send_string(ews_t * ews, char const * msg)
{
	int n = strlen(msg);
	return ews_send_message(ews, msg, n);
}