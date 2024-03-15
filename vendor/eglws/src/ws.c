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
#include "eglws_vhd.h"
#include "msg.h"





/* this runs under the lws service thread context only */
static int _callback_minimal(struct lws *wsi, enum lws_callback_reasons reason,void *user, void *in, size_t len)
{
	eglws_pss_t *pss = (eglws_pss_t *)user;
	eglws_vhd_t *vhd = (eglws_vhd_t *)lws_protocol_vh_priv_get(lws_get_vhost(wsi), lws_get_protocol(wsi));
	
	

	
	char ip[64];
	int fd = lws_get_socket_fd(wsi);
	if ((fd > 0) && lws_get_peer_simple(wsi, ip, 16)) {
		printf("reason: %s, fd:%i, ip:%s\n", lws_callback_reasons_tostr(reason), fd, ip);
	} else {
		printf("reason: %s, fd:%i\n", lws_callback_reasons_tostr(reason), fd);
	}
	

	int rc = 0;

	switch (reason) {
	case LWS_CALLBACK_PROTOCOL_INIT:
		assert(vhd == NULL);
		vhd = eglws_vhd_init(wsi, in);
		if(vhd == NULL) {
			return 1;
		} else {
			ews_t * a = lws_vhost_user(lws_get_vhost(wsi));
			a->internal_vhd = vhd;
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
		{
			char buf[128] = {0};
			snprintf(buf, sizeof(buf), "[NEW_SESSION]: %s", ip);
			eglws_vhd_send_text(vhd, buf);
		}
		break;

	case LWS_CALLBACK_CLOSED:
		/* remove our closing pss from the list of live pss */
		lws_ll_fwd_remove(eglws_pss_t, pss_list, pss, vhd->pss_list);
		break;

	case LWS_CALLBACK_SERVER_WRITEABLE:
		rc = eglws_vhd_consume(vhd, pss, wsi);
		break;

	case LWS_CALLBACK_RECEIVE:
		lwsl_user("LWS_CALLBACK_RECEIVE: %4d (rpp %5d, first %d, ""last %d, bin %d, len %d)\n",(int)len, (int)lws_remaining_packet_payload(wsi),lws_is_first_fragment(wsi),lws_is_final_fragment(wsi),lws_frame_is_binary(wsi), (int)len);
		//rc = eglws_vhd_send_message(vhd, in, len);
		break;

	case LWS_CALLBACK_EVENT_WAIT_CANCELLED:
		rc = eglws_vhd_request_writable(vhd);
		break;

	default:
		break;
	}

	return rc;
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


int ews_send_binary(ews_t * ews, void const * data, int len)
{
	return eglws_vhd_send_binary(ews->internal_vhd, data, len);
}


int ews_send_text(ews_t * ews, char const * msg)
{
	return eglws_vhd_send_text(ews->internal_vhd, msg);
}